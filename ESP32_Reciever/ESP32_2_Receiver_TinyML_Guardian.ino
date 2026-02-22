#include <mcp_can.h>
#include <SPI.h>

/*
 * OBD-II Reader / TinyML Processor
 * --------------------------------
 * 1. Receives CAN messages from Vehicle Simulator
 * 2. Parses float values
 * 3. Runs TinyML Inference
 */

#include "model_a_safety.h"
#include "model_b_health.h"
#include "model_c_driver.h"
#include "model_utils.h"

// Hardware Config
#define CAN0_INT 21  // Set INT to pin 21 (or 4 on some boards)
MCP_CAN CAN0(5);     // Set CS to pin 5

// State Variables (Received from CAN)
SafetySensors safety_in;
HealthSensors health_in;
DriverSensors driver_in;

// Classifiers
SafetyModel::Eloquent::ML::Port::RandomForest safety_ai;
HealthModel::Eloquent::ML::Port::RandomForest health_ai;
DriverModel::Eloquent::ML::Port::RandomForest driver_ai;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

void setup() {
    Serial.begin(115200);
    
    // Init CAN
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Init OK!");
    else Serial.println("MCP2515 Init Failed...");
    
    CAN0.setMode(MCP_NORMAL);
    pinMode(CAN0_INT, INPUT);
    
    Serial.println("TinyML Diagnostic System Ready. Waiting for CAN data...");
}

void loop() {
    // Check for CAN Message
    if(!digitalRead(CAN0_INT)) {
        CAN0.readMsgBuf(&rxId, &len, rxBuf);
        
        // Parse Float Pairs
        float val1, val2;
        memcpy(&val1, &rxBuf[0], 4);
        memcpy(&val2, &rxBuf[4], 4);
        
        // Store Data based on CAN ID
        switch(rxId) {
            // --- SAFETY ---
            case 0x100: safety_in.pack_voltage = val1; safety_in.pack_current = val2; break;
            case 0x101: safety_in.cell_max_v = val1; safety_in.cell_min_v = val2; break;
            case 0x102: safety_in.pack_temp = val1; safety_in.inverter_temp = val2; break;
            case 0x103: safety_in.ambient_temp = val1; safety_in.coolant_flow = val2; break;
            case 0x104: safety_in.iso_resistance = val1; safety_in.gas_ppm = val2; break;
            
            // --- HEALTH ---
            case 0x200: health_in.soc = val1; health_in.internal_res = val2; break;
            case 0x201: health_in.cycle_count = val1; health_in.dod = val2; break;
            case 0x202: health_in.cell_imbalance = val1; health_in.coulombic_eff = val2; break;
            case 0x203: health_in.pol_voltage = val1; health_in.stress_index = val2; break;
            
            // --- DRIVER ---
            case 0x300: driver_in.speed_avg = val1; driver_in.brake_freq = val2; break;
            case 0x301: driver_in.brake_intensity = val1; driver_in.throttle_var = val2; break;
            case 0x302: driver_in.energy_consumption = val1; driver_in.range_est = val2; break;
        }
        
        // Run Inference periodically (e.g., every full cycle or time-based)
        static unsigned long last_pred = 0;
        if (millis() - last_pred > 2000) {
            last_pred = millis();
            run_diagnostics();
        }
    }
}

void run_diagnostics() {
    Serial.println("\n--- [Running AI Diagnostics] ---");
    
    // 1. Safety
    float s_feat[13];
    extract_safety_features(safety_in, s_feat);
    int s_res = safety_ai.predict(s_feat);
    Serial.print("SAFETY:  "); Serial.println(get_safety_label(s_res));
    
    // 2. Health
    float h_feat[8];
    extract_health_features(health_in, h_feat);
    int h_res = health_ai.predict(h_feat);
    Serial.print("HEALTH:  "); Serial.println(get_health_label(h_res));
    
    // 3. Driver
    float d_feat[6];
    extract_driver_features(driver_in, d_feat);
    int d_res = driver_ai.predict(d_feat);
    Serial.print("DRIVER:  "); Serial.println(get_driver_label(d_res));
    
    Serial.println("--------------------------------");
}
