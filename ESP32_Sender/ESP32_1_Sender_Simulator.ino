#include <SPI.h>
#include <mcp_can.h>
#include "sim_config.h"

// ==========================================
// SIMULATOR CONFIGURATION
// ==========================================

// Global state variables
struct VehicleState {
    float pack_voltage = 350.0;
    float pack_current = 0.0;
    float cell_max_v = 4.15;
    float cell_min_v = 4.10;
    float pack_temp = 30.0;
    float inverter_temp = 35.0;
    float ambient_temp = 25.0;
    float coolant_flow = 8.0;
    float iso_resistance = 500.0;
    float gas_ppm = 0.0;

    float soc = 80.0;
    float internal_res = 60.0;
    float cycle_count = 400.0;
    float dod = 60.0;
    float cell_imbalance = 0.05;
    float coulombic_eff = 99.0;
    float pol_voltage = 0.05;
    float stress_index = 1.0;

    float speed_avg = 0.0;
    float brake_freq = 0.0;
    float brake_intensity = 0.0;
    float throttle_var = 0.0;
    float energy_consumption = 0.0;
    float range_est = 200.0;
} vehicle;

// Simulation Modes
enum SimMode {
    MODE_IDLE,
    MODE_CITY,
    MODE_HIGHWAY,
    MODE_EMERGENCY,
    MODE_FAULT_THERMAL,
    MODE_FAULT_ELEC
};

SimMode currentMode = MODE_IDLE;
unsigned long lastUpdate = 0;
MCP_CAN CAN0(CAN_CS_PIN); // Set CS to pin 5

// ==========================================
// CAN TRANSMISSION HELPERS
// ==========================================

void sendFloat(unsigned long id, float val1, float val2) {
    byte data[8];
    memcpy(&data[0], &val1, 4);
    memcpy(&data[4], &val2, 4);
    
    // Send standard frame
    byte sndStat = CAN0.sendMsgBuf(id, 0, 8, data);
    
    if(sndStat == CAN_OK){
        // Serial.println("Message Sent Successfully!");
    } else {
        Serial.print("Error Sending Message... ID: 0x");
        Serial.println(id, HEX);
    }
}

// ==========================================
// SIMULATION LOGIC
// ==========================================

void updateVehiclePhysics() {
    // Basic Physics Simulation based on Mode
    
    switch (currentMode) {
        case MODE_IDLE:
            vehicle.speed_avg = 0.0;
            vehicle.pack_current = 2.0; // Minimal load (HVAC?)
            vehicle.throttle_var = 0.0;
            vehicle.brake_intensity = 0.0;
            vehicle.pack_temp = max(25.0, vehicle.pack_temp - 0.1); // Cool down
            break;

        case MODE_CITY:
            // Stop-and-Go simulation
            vehicle.speed_avg = 25.0 + random(-10, 15);
            vehicle.pack_current = random(20, 80);
            vehicle.brake_freq = random(10, 20); // High brake frequency
            vehicle.brake_intensity = random(10, 40);
            vehicle.throttle_var = random(15, 30);
            vehicle.energy_consumption = 0.15 + (random(-2, 3) / 100.0);
            vehicle.pack_temp += 0.05; // Slow heat up
            break;

        case MODE_HIGHWAY:
            // Steady driving
            vehicle.speed_avg = 80.0 + random(-5, 10);
            vehicle.pack_current = random(40, 60); // Steady load
            vehicle.brake_freq = random(0, 3); // Low braking
            vehicle.brake_intensity = random(0, 10);
            vehicle.throttle_var = random(2, 8); // Smooth throttle
            vehicle.energy_consumption = 0.12 + (random(-1, 2) / 100.0);
            vehicle.pack_temp += 0.02; // Very slow heat up (good airflow)
            break;

        case MODE_EMERGENCY:
            // Panic simulation
            vehicle.speed_avg = random(10, 90); // Could be any speed
            vehicle.pack_current = random(100, 200); // High discharge then regen?
            vehicle.brake_freq = random(5, 10);
            vehicle.brake_intensity = random(85, 100); // MAX BRAKING
            vehicle.throttle_var = random(40, 60); // Erratic
            vehicle.stress_index += 0.1; // Increasing stress
            break;

        case MODE_FAULT_THERMAL:
            // Thermal Runaway simulation
            vehicle.pack_temp += 2.0; // Rapid heating!
            vehicle.inverter_temp += 1.5;
            vehicle.coolant_flow = max(0.0, vehicle.coolant_flow - 0.5); // Pump failure?
            vehicle.gas_ppm += 5.0; // Venting
            vehicle.iso_resistance = 500.0; // Iso might be fine initially
            break;
            
        case MODE_FAULT_ELEC:
            // Electrical Fault
            vehicle.iso_resistance = max(0.0, vehicle.iso_resistance - 50.0); // Ground fault
            vehicle.cell_imbalance = 0.5; // Major cell delta
            vehicle.pack_current = random(0, 10); // Contactor open/flickering
            break;
    }

    // Generic updates
    vehicle.pack_voltage = 350.0 - (vehicle.pack_current * 0.05); // Sag
    vehicle.soc = max(0.0, vehicle.soc - (vehicle.pack_current * 0.0001)); // Discharge
    vehicle.range_est = vehicle.soc * 2.5; // Simple range calc

    // Bounds checking
    if (vehicle.pack_temp > 100) vehicle.pack_temp = 100;
    if (vehicle.gas_ppm > 1000) vehicle.gas_ppm = 1000;
}

void transmitData() {
    // Send Safety Data (ID 0x100 - 0x102)
    sendFloat(CAN_ID_SAFETY_1, vehicle.pack_voltage, vehicle.pack_current);
    // Note: We need to pack 4 floats into 2 messages or define a custom struct.
    // For simplicity, we send specific pairs.
    
    // We need to send:
    // Safety: [PackV, PackI], [CellMax, CellMin], [PackTemp, InvTemp], [AmbTemp, Coolant], [IsoRes, GasPPM]
    // That's 5 messages. Let's expand the IDs slightly or cycle them.
    
    // Safety
    sendFloat(0x100, vehicle.pack_voltage, vehicle.pack_current);
    sendFloat(0x101, vehicle.cell_max_v, vehicle.cell_min_v);
    sendFloat(0x102, vehicle.pack_temp, vehicle.inverter_temp);
    sendFloat(0x103, vehicle.ambient_temp, vehicle.coolant_flow);
    sendFloat(0x104, vehicle.iso_resistance, vehicle.gas_ppm);

    // Health (ID 0x200+)
    // [SoC, InternalRes], [Cycles, DoD], [Imbalance, CoulEff], [PolV, Stress]
    sendFloat(0x200, vehicle.soc, vehicle.internal_res);
    sendFloat(0x201, vehicle.cycle_count, vehicle.dod);
    sendFloat(0x202, vehicle.cell_imbalance, vehicle.coulombic_eff);
    sendFloat(0x203, vehicle.pol_voltage, vehicle.stress_index);

    // Driver (ID 0x300+)
    // [Speed, BrakeFreq], [BrakeInt, ThrottleVar], [Energy, Range]
    sendFloat(0x300, vehicle.speed_avg, vehicle.brake_freq);
    sendFloat(0x301, vehicle.brake_intensity, vehicle.throttle_var);
    sendFloat(0x302, vehicle.energy_consumption, vehicle.range_est);
}

// ==========================================
// SETUP & LOOP
// ==========================================

void setup() {
    Serial.begin(115200);
    
    // Initialize MCP2515
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
        Serial.println("MCP2515 Initialized Successfully!");
    else
        Serial.println("Error Initializing MCP2515...");
        
    CAN0.setMode(MCP_NORMAL);

    Serial.println("
=== VEHICLE SIMULATOR STARTED ===");
    Serial.println("Commands:");
    Serial.println(" [1] Idle Mode");
    Serial.println(" [2] City Driving");
    Serial.println(" [3] Highway Driving");
    Serial.println(" [4] Emergency Event");
    Serial.println(" [5] FAULT: Thermal Runaway");
    Serial.println(" [6] FAULT: Electrical Short");
}

void loop() {
    // Check Serial Input for Mode Switching
    if (Serial.available()) {
        char cmd = Serial.read();
        switch(cmd) {
            case '1': currentMode = MODE_IDLE; Serial.println("-> MODE: IDLE"); break;
            case '2': currentMode = MODE_CITY; Serial.println("-> MODE: CITY"); break;
            case '3': currentMode = MODE_HIGHWAY; Serial.println("-> MODE: HIGHWAY"); break;
            case '4': currentMode = MODE_EMERGENCY; Serial.println("-> MODE: EMERGENCY"); break;
            case '5': currentMode = MODE_FAULT_THERMAL; Serial.println("-> MODE: FAULT (Thermal)"); break;
            case '6': currentMode = MODE_FAULT_ELEC; Serial.println("-> MODE: FAULT (Electrical)"); break;
        }
    }

    // Update Physics & Transmit
    if (millis() - lastUpdate > UPDATE_RATE_MS) {
        lastUpdate = millis();
        updateVehiclePhysics();
        transmitData();
        
        // Debug Print
        Serial.print("Mode: "); Serial.print(currentMode);
        Serial.print(" | Speed: "); Serial.print(vehicle.speed_avg);
        Serial.print(" | Temp: "); Serial.print(vehicle.pack_temp);
        Serial.println();
    }
}
