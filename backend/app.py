"""
Smart Bus Overcrowding Detection & Alert System - Cloud Backend API
Institution: Muthoot Institute of Technology and Science (MITS Kochi)
Department: Department of Artificial Intelligence & Data Science
Authors: Edwin Thomas (29), Adwaith Vimal (12), Bevan Bince (26), Joswin Thomas (40)

Tech Stack: Python, Flask / FastAPI, SQLite / PostgreSQL, MQTT Subscriber
"""

import os
import json
import sqlite3
import datetime
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

DB_PATH = os.path.join(os.path.dirname(__file__), "smart_bus.db")

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    # Bus telemetry history
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS occupancy_logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        bus_id TEXT NOT NULL,
        occupancy INTEGER NOT NULL,
        max_capacity INTEGER NOT NULL,
        occupancy_pct REAL NOT NULL,
        status TEXT NOT NULL,
        boarded_delta INTEGER DEFAULT 0,
        alighted_delta INTEGER DEFAULT 0,
        stop_name TEXT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    )
    """)
    
    # Overcrowding alert incidents
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS overcrowding_alerts (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        bus_id TEXT NOT NULL,
        occupancy INTEGER NOT NULL,
        max_capacity INTEGER NOT NULL,
        severity TEXT NOT NULL,
        message TEXT NOT NULL,
        acknowledged BOOLEAN DEFAULT 0,
        acknowledged_by TEXT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    )
    """)

    # Fleet information
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS fleet_buses (
        bus_id TEXT PRIMARY KEY,
        route_name TEXT,
        driver_name TEXT,
        max_capacity INTEGER DEFAULT 45,
        current_occupancy INTEGER DEFAULT 0,
        status TEXT DEFAULT 'NORMAL',
        last_updated DATETIME DEFAULT CURRENT_TIMESTAMP
    )
    """)
    
    # Seed default bus
    cursor.execute("""
    INSERT OR REPLACE INTO fleet_buses (bus_id, route_name, driver_name, max_capacity, current_occupancy, status, last_updated)
    VALUES ('KL-07-MITS-01', 'Route 42: Aluva Metro -> Kakkanad InfoPark', 'Rajesh Kumar', 45, 0, 'NORMAL', CURRENT_TIMESTAMP)
    """)

    conn.commit()
    conn.close()

init_db()

@app.route("/api/health", methods=["GET"])
def health_check():
    return jsonify({
        "system": "Smart Bus Overcrowding Backend",
        "status": "ONLINE",
        "version": "1.0.0",
        "institution": "MITS Kochi - Dept. of AI & DS",
        "timestamp": datetime.datetime.now().isoformat()
    })

@app.route("/api/telemetry", methods=["POST"])
def receive_telemetry():
    """
    Ingests live sensor telemetry from Arduino / ESP32 MQTT bridge
    Payload format:
    {
        "bus_id": "KL-07-MITS-01",
        "occupancy": 46,
        "max_capacity": 45,
        "boarded": 12,
        "alighted": 2,
        "status": "OVERCROWDED",
        "stop_name": "Edappally Toll"
    }
    """
    data = request.get_json(force=True)
    bus_id = data.get("bus_id", "KL-07-MITS-01")
    occupancy = int(data.get("occupancy", 0))
    max_cap = int(data.get("max_capacity", 45))
    status = data.get("status", "NORMAL")
    boarded = int(data.get("boarded", 0))
    alighted = int(data.get("alighted", 0))
    stop_name = data.get("stop_name", "En Route")
    pct = round((occupancy / max_cap) * 100.0, 1) if max_cap > 0 else 0

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    cursor.execute("""
        INSERT INTO occupancy_logs (bus_id, occupancy, max_capacity, occupancy_pct, status, boarded_delta, alighted_delta, stop_name)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    """, (bus_id, occupancy, max_cap, pct, status, boarded, alighted, stop_name))

    # Trigger alert record if overcrowding occurred
    alert_triggered = False
    if occupancy >= max_cap:
        alert_triggered = True
        cursor.execute("""
            INSERT INTO overcrowding_alerts (bus_id, occupancy, max_capacity, severity, message)
            VALUES (?, ?, ?, 'CRITICAL', ?)
        """, (bus_id, occupancy, max_cap, f"Bus {bus_id} exceeded safe capacity ({occupancy}/{max_cap}) at {stop_name}"))

    cursor.execute("""
        UPDATE fleet_buses
        SET current_occupancy = ?, status = ?, last_updated = CURRENT_TIMESTAMP
        WHERE bus_id = ?
    """, (occupancy, status, bus_id))

    conn.commit()
    conn.close()

    return jsonify({
        "success": True,
        "alert_triggered": alert_triggered,
        "occupancy": occupancy,
        "occupancy_pct": pct,
        "status": status,
        "processed_at": datetime.datetime.now().isoformat()
    }), 201

@app.route("/api/fleet", methods=["GET"])
def get_fleet_status():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM fleet_buses")
    rows = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return jsonify({"fleet": rows})

@app.route("/api/alerts", methods=["GET"])
def get_alerts():
    limit = int(request.args.get("limit", 50))
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM overcrowding_alerts ORDER BY id DESC LIMIT ?", (limit,))
    rows = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return jsonify({"alerts": rows})

@app.route("/api/analytics/history", methods=["GET"])
def get_history():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM occupancy_logs ORDER BY id DESC LIMIT 100")
    rows = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return jsonify({"history": rows[::-1]})

@app.route("/api/alerts/acknowledge/<int:alert_id>", methods=["POST"])
def acknowledge_alert(alert_id):
    req_data = request.get_json(silent=True) or {}
    ack_by = req_data.get("acknowledged_by", "Authority Officer")
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("UPDATE overcrowding_alerts SET acknowledged = 1, acknowledged_by = ? WHERE id = ?", (ack_by, alert_id))
    conn.commit()
    conn.close()
    return jsonify({"success": True, "alert_id": alert_id, "status": "ACKNOWLEDGED"})

if __name__ == "__main__":
    print("[FLASK API] Running Smart Bus Overcrowding Backend on http://0.0.0.0:5000 ...")
    app.run(host="0.0.0.0", port=5000, debug=True)
