import numpy as np
import pandas as pd

np.random.seed(42)

print("="*80)
print("GENERATING NOISY DATA (REALISTIC SCENARIOS)")
print("Targeting ~85-90% Accuracy by introducing overlaps and sensor errors")
print("="*80)

# ============================================
# HELPER: ADD SENSOR NOISE
# ============================================
def add_noise(value, noise_level=0.05):
    """Adds random percentage noise to a value"""
    noise = np.random.normal(0, abs(value * noise_level))
    return value + noise

# ============================================
# MODEL A - SAFETY (ANOMALY DETECTION)
# Labels: 0 (Normal), 1 (Warning), 2 (Critical)
# ============================================
print("\n[1/3] Generating Model A - Safety...")

def generate_safety_data(n_samples=10000):
    data = []
    classes = [0, 1, 2]
    
    # Probabilistic label assignment (imbalanced classes in real life, but balanced here for training)
    labels = np.random.choice(classes, n_samples)
    
    for label in labels:
        # 1. Base Physics (With significant overlap)
        if label == 0: # Normal
            pack_temp = np.random.normal(35, 12)       # Wider range (was 8)
            iso_res = np.random.normal(420, 120)       # More overlap (was 450, 100)
            gas_ppm = np.random.exponential(10)        # Higher noise
            cell_delta = np.random.normal(0.03, 0.03)  
            coolant_flow = np.random.normal(8, 3)      
            
        elif label == 1: # Warning (Hard to distinguish from high-load Normal)
            pack_temp = np.random.normal(50, 15)       # Closer to Normal (was 55)
            iso_res = np.random.normal(350, 140)       # closer to Normal (was 300)
            gas_ppm = np.random.normal(30, 25)         # Noisy sensor
            cell_delta = np.random.normal(0.08, 0.06)  # Closer to Normal (was 0.10)
            coolant_flow = np.random.normal(6, 4)      
            
        else: # Critical
            pack_temp = np.random.normal(75, 25)       # Overlaps with Warning
            iso_res = np.random.normal(100, 100)        
            gas_ppm = np.random.normal(150, 100)        
            cell_delta = np.random.normal(0.35, 0.20)  
            coolant_flow = np.random.normal(3, 4)      

        # 2. Add "Sensor Glitches" (10% of data has outlier values)
        if np.random.random() < 0.10:
            pack_temp += np.random.choice([-30, 30]) # Faulty temp sensor
        
        # 3. Label Noise (10% mislabeled by human annotators)
        if np.random.random() < 0.10:
            label = np.random.choice([0, 1, 2])

        # Feature Engineering
        pack_voltage = np.random.normal(350, 15)
        pack_current = np.random.normal(60, 40)
        
        # Derived features with noise
        instant_power = (pack_voltage * pack_current) / 1000
        instant_power = add_noise(instant_power, 0.2) # 20% calculation error

        data.append({
            'Pack_Voltage': round(abs(pack_voltage), 2),
            'Pack_Current': round(abs(pack_current), 2),
            'Instant_Power': round(abs(instant_power), 2),
            'Cell_Max': round(4.2 - abs(np.random.normal(0, 0.05)), 2),
            'Cell_Min': round(4.2 - abs(np.random.normal(0, 0.05)) - abs(cell_delta), 2),
            'Pack_Temp': round(max(0, pack_temp), 2),
            'Thermal_Grad': round(max(0, abs(cell_delta * 5) + np.random.normal(0, 1.0)), 2),
            'Inverter_Temp': round(max(0, pack_temp + np.random.normal(5, 5)), 2),
            'Ambient_Temp': round(np.random.normal(25, 10), 2),
            'Coolant_Flow': round(max(0, coolant_flow), 2),
            'Iso_Resistance': round(max(0, iso_res), 2),
            'Gas_PPM': round(max(0, gas_ppm), 1),
            'SoP': round(max(0, 150 - (pack_temp * 0.8) + np.random.normal(0, 10)), 1),
            'Label_Safety': label
        })
    
    return pd.DataFrame(data)

df_safety = generate_safety_data(10000)
df_safety.to_csv('model_a_safety_10k.csv', index=False)
print(f"✓ Generated {len(df_safety):,} noisy samples for Safety")


# ============================================
# MODEL B - HEALTH (SOH ESTIMATION)
# Labels: 0 (Good), 1 (Bad)
# ============================================
print("\n[2/3] Generating Model B - Health...")

def generate_health_data(n_samples=10000):
    data = []
    classes = [0, 1]
    
    for _ in range(n_samples):
        # latent variable: Real SOH (State of Health)
        real_soh = np.random.uniform(60, 100)
        
        # Probabilistic Labelling based on SOH
        # Threshold is roughly 80%, but it's fuzzy
        # SOH 82% might be labeled "Bad" (conservative)
        # SOH 78% might be labeled "Good" (optimistic)
        prob_good = 1 / (1 + np.exp(-(real_soh - 80) / 2)) # Sigmoid function centered at 80
        label = 0 if np.random.random() < prob_good else 1
        
        # Features derived from Real SOH + High Noise
        cycle_count = int(max(0, (100 - real_soh) * 50 + np.random.normal(0, 500)))
        internal_res = max(50, (150 - real_soh) + np.random.normal(0, 20))
        
        # Ambiguous Feature: DoD doesn't always correlate perfectly
        dod = np.random.uniform(10, 90) 
        
        data.append({
            'SoC': round(np.random.uniform(10, 100), 1),
            'Internal_Res': round(internal_res, 1),
            'Cycle_Count': cycle_count,
            'DoD': round(dod, 2),
            'Cell_Imbalance': round(max(0, (100-real_soh)/500 + np.random.normal(0, 0.05)), 3),
            'Coulombic_Eff': round(min(100, 95 + (real_soh/25) + np.random.normal(0, 1.0)), 2),
            'Pol_Voltage': round(max(0, (100-real_soh)/200 + np.random.normal(0, 0.05)), 2),
            'Stress_Index': round(max(0, (cycle_count/1000) + np.random.normal(0, 0.5)), 2),
            'Label_Health': label
        })
    
    return pd.DataFrame(data)

df_health = generate_health_data(10000)
df_health.to_csv('model_b_health_10k.csv', index=False)
print(f"✓ Generated {len(df_health):,} noisy samples for Health")


# ============================================
# MODEL C - DRIVER (INDIAN ROADS)
# Labels: 0 (City), 1 (Highway), 2 (Emergency)
# ============================================
print("\n[3/3] Generating Model C - Driver (Indian Context)...")

def generate_driver_data(n_samples=10000):
    data = []
    classes = [0, 1, 2]
    
    for label in np.random.choice(classes, n_samples, p=[0.5, 0.4, 0.1]):
        
        # SCENARIO OVERLAPS:
        # 1. "Highway Traffic Jam": High brake freq, low speed (Looks like City)
        # 2. "Empty Night City Road": High speed, low brake (Looks like Highway)
        # 3. "Aggressive City Driving": Hard braking (Looks like Emergency)
        
        is_traffic_jam = (label == 1 and np.random.random() < 0.15)
        is_empty_city = (label == 0 and np.random.random() < 0.10)
        is_panic_city = (label == 0 and np.random.random() < 0.05)

        if label == 0: # City
            if is_empty_city:
                speed_avg = np.random.normal(60, 10) # Fast!
                brake_freq = np.random.normal(5, 3)
            elif is_panic_city:
                speed_avg = np.random.normal(30, 10)
                brake_freq = np.random.normal(20, 5) # Panic!
            else:
                speed_avg = np.random.normal(30, 15)
                brake_freq = np.random.normal(15, 8)
                
            throttle_std = np.random.normal(20, 10)
            brake_intensity = np.random.normal(35, 15)

        elif label == 1: # Highway
            if is_traffic_jam:
                speed_avg = np.random.normal(20, 10) # Slow!
                brake_freq = np.random.normal(12, 5) # Stop-and-go
            else:
                speed_avg = np.random.normal(65, 20) # Wide range
                brake_freq = np.random.normal(3, 3)
            
            throttle_std = np.random.normal(10, 5)
            brake_intensity = np.random.normal(15, 10)

        else: # Emergency
            # Can happen at ANY speed
            speed_avg = np.random.uniform(10, 100) 
            brake_freq = np.random.normal(5, 5)
            throttle_std = np.random.normal(50, 20)
            brake_intensity = np.random.normal(85, 15) # High, but can dip to 70

        # Add generic sensor noise
        speed_avg = add_noise(speed_avg, 0.1)
        
        # Labels are sometimes wrong (e.g. driver just braked hard for a pothole, not emergency)
        if np.random.random() < 0.05:
             label = np.random.choice([0, 1, 2])

        data.append({
            'Speed_Avg': round(max(0, speed_avg), 1),
            'Brake_Frequency': round(max(0, brake_freq), 1),
            'Brake_Intensity': round(max(0, min(100, brake_intensity)), 1),
            'Throttle_Variance': round(max(0, throttle_std), 2),
            'Energy_Consumption': round(max(0, np.random.normal(0.15, 0.05)), 3), # Very noisy
            'Range_Est': round(max(0, np.random.normal(150, 40)), 1), # Almost random
            'Label_Driver': label
        })
    
    return pd.DataFrame(data)

df_driver = generate_driver_data(10000)
df_driver.to_csv('model_c_driver_10k.csv', index=False)
print(f"✓ Generated {len(df_driver):,} noisy samples for Driver (Indian Context)")

print("\n" + "="*80)
print("DATA GENERATION COMPLETE")
print("="*80)
