-- PostgreSQL / SQLite Database Schema
-- Smart Bus Overcrowding Detection & Alert System
-- Department of AI & DS, Muthoot Institute of Technology and Science (MITS)

CREATE TABLE IF NOT EXISTS fleet_buses (
    bus_id VARCHAR(32) PRIMARY KEY,
    route_name VARCHAR(128) NOT NULL,
    driver_name VARCHAR(64) NOT NULL,
    max_capacity INT DEFAULT 45,
    current_occupancy INT DEFAULT 0,
    status VARCHAR(16) DEFAULT 'NORMAL',
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS occupancy_logs (
    id SERIAL PRIMARY KEY,
    bus_id VARCHAR(32) REFERENCES fleet_buses(bus_id),
    occupancy INT NOT NULL,
    max_capacity INT NOT NULL,
    occupancy_pct NUMERIC(5,2) NOT NULL,
    status VARCHAR(16) NOT NULL,
    boarded_delta INT DEFAULT 0,
    alighted_delta INT DEFAULT 0,
    stop_name VARCHAR(64),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS overcrowding_alerts (
    id SERIAL PRIMARY KEY,
    bus_id VARCHAR(32) REFERENCES fleet_buses(bus_id),
    occupancy INT NOT NULL,
    max_capacity INT NOT NULL,
    severity VARCHAR(16) DEFAULT 'CRITICAL',
    message TEXT NOT NULL,
    acknowledged BOOLEAN DEFAULT FALSE,
    acknowledged_by VARCHAR(64),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Seed initial route data
INSERT INTO fleet_buses (bus_id, route_name, driver_name, max_capacity, current_occupancy, status)
VALUES ('KL-07-MITS-01', 'Route 42: Aluva Metro -> Kakkanad InfoPark', 'Rajesh Kumar', 45, 0, 'NORMAL')
ON CONFLICT (bus_id) DO NOTHING;
