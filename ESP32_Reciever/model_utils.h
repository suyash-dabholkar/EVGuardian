#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include <Arduino.h>

// ==========================================
// DATA STRUCTURES (RAW SENSOR INPUTS)
// ==========================================

struct SafetySensors {
    float pack_voltage;      // V
    float pack_current;      // A
    float cell_max_v;        // V
    float cell_min_v;        // V
    float pack_temp;         // C
    float inverter_temp;     // C
    float ambient_temp;      // C
    float coolant_flow;      // L/min
    float iso_resistance;    // kOhm
    float gas_ppm;           // ppm
};

struct HealthSensors {
    float soc;               // %
    float internal_res;      // mOhm
    float cycle_count;       // count
    float dod;               // % (Depth of Discharge)
    float cell_imbalance;    // V (delta)
    float coulombic_eff;     // %
    float pol_voltage;       // V
    float stress_index;      // 0-10 scale
};

struct DriverSensors {
    float speed_avg;         // km/h
    float brake_freq;        // events/min
    float brake_intensity;   // % pressure
    float throttle_var;      // variance
    float energy_consumption;// kWh/km
    float range_est;         // km
};

// ==========================================
// FEATURE EXTRACTION HELPERS
// ==========================================

/**
 * Calculates derived features for the Safety Model
 * Input: Raw sensor struct
 * Output: Feature array [13] expected by the model
 */
void extract_safety_features(SafetySensors data, float* features) {
    // 1. Pack_Voltage
    features[0] = data.pack_voltage;
    
    // 2. Pack_Current
    features[1] = data.pack_current;
    
    // 3. Instant_Power (Derived) -> kW
    features[2] = (data.pack_voltage * data.pack_current) / 1000.0;
    
    // 4. Cell_Max
    features[3] = data.cell_max_v;
    
    // 5. Cell_Min
    features[4] = data.cell_min_v;
    
    // 6. Pack_Temp
    features[5] = data.pack_temp;
    
    // 7. Thermal_Grad (Derived) -> Approximation
    // In real system: delta between hottest and coldest sensor
    features[6] = (data.cell_max_v - data.cell_min_v) * 5.0; // Simplified estimation
    
    // 8. Inverter_Temp
    features[7] = data.inverter_temp;
    
    // 9. Ambient_Temp
    features[8] = data.ambient_temp;
    
    // 10. Coolant_Flow
    features[9] = data.coolant_flow;
    
    // 11. Iso_Resistance
    features[10] = data.iso_resistance;
    
    // 12. Gas_PPM
    features[11] = data.gas_ppm;
    
    // 13. SoP (State of Power) -> Derived/Estimated
    // Simple heuristic: reduced by low temp, low voltage, or high temp
    float temp_derating = max(0.0, (double)(data.pack_temp - 45.0) * 2.0);
    features[12] = max(0.0, 150.0 - temp_derating); 
}

/**
 * Prepares features for the Health Model
 * Input: Raw sensor struct
 * Output: Feature array [8]
 */
void extract_health_features(HealthSensors data, float* features) {
    features[0] = data.soc;
    features[1] = data.internal_res;
    features[2] = data.cycle_count;
    features[3] = data.dod;
    features[4] = data.cell_imbalance;
    features[5] = data.coulombic_eff;
    features[6] = data.pol_voltage;
    features[7] = data.stress_index;
}

/**
 * Prepares features for the Driver Model
 * Input: Raw sensor struct
 * Output: Feature array [6]
 */
void extract_driver_features(DriverSensors data, float* features) {
    features[0] = data.speed_avg;
    features[1] = data.brake_freq;
    features[2] = data.brake_intensity;
    features[3] = data.throttle_var;
    features[4] = data.energy_consumption;
    features[5] = data.range_est;
}

// ==========================================
// PREDICTION INTERPRETERS
// ==========================================

const char* get_safety_label(int class_idx) {
    switch(class_idx) {
        case 0: return "NORMAL";
        case 1: return "WARNING (Thermal/Elec Risk)";
        case 2: return "CRITICAL FAILURE";
        default: return "UNKNOWN";
    }
}

const char* get_health_label(int class_idx) {
    return (class_idx == 0) ? "GOOD" : "BAD (Replace/Service)";
}

const char* get_driver_label(int class_idx) {
    switch(class_idx) {
        case 0: return "CITY (Stop-and-Go)";
        case 1: return "HIGHWAY (Steady)";
        case 2: return "EMERGENCY (Panic)";
        default: return "UNKNOWN";
    }
}

#endif
