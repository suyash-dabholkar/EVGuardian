import joblib
from micromlgen import port
import os

OUTPUT_DIR = '../ESP32_2_Receiver_TinyML_Guardian'

# Ensure output directory exists
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

def convert_and_save(model_path, header_name, class_map, namespace):
    print(f"Converting {model_path}...")
    
    try:
        # Load model
        clf = joblib.load(model_path)
        
        # Convert to C++ code
        c_code = port(clf, classmap=class_map)
        
        # Wrap in namespace to avoid conflicts on ESP32
        # (micromlgen uses the same class names by default)
        final_code = f"""
#ifndef {header_name.upper()}_H
#define {header_name.upper()}_H

namespace {namespace} {{
    {c_code}
}}

#endif // {header_name.upper()}_H
"""
        
        # Save to file
        output_path = f'{OUTPUT_DIR}/{header_name}.h'
        with open(output_path, 'w') as f:
            f.write(final_code)
            
        print(f"✓ Saved to {output_path}")
        
    except Exception as e:
        print(f"❌ Error converting {model_path}: {e}")

# ============================================
# 1. Model A - Safety
# ============================================
convert_and_save(
    'models/model_a_safety.pkl', 
    'model_a_safety', 
    {0: 'Normal', 1: 'Warning', 2: 'Critical'},
    'SafetyModel'
)

# ============================================
# 2. Model B - Health
# ============================================
convert_and_save(
    'models/model_b_health.pkl', 
    'model_b_health', 
    {0: 'Good', 1: 'Bad'},
    'HealthModel'
)

# ============================================
# 3. Model C - Driver
# ============================================
convert_and_save(
    'models/model_c_driver.pkl', 
    'model_c_driver', 
    {0: 'City', 1: 'Highway', 2: 'Emergency'},
    'DriverModel'
)
print(f"\nConversion Complete! Headers saved to '{OUTPUT_DIR}'.")