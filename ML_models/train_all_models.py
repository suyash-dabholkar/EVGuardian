import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
import joblib
import os

# Create models directory if it doesn't exist
if not os.path.exists('models'):
    os.makedirs('models')

print("="*80)
print("TRAINING ML MODELS FOR BATTERY DIAGNOSTICS (TINYML OPTIMIZED)")
print("="*80)
print("Using smaller models (n_estimators=20, max_depth=10) for ESP32 deployment")

# ============================================
# 1. TRAIN MODEL A - SAFETY (Anomaly Detection)
# ============================================
print("\n[1/3] Training Model A - Safety Classification...")

try:
    # Load Data
    df_safety = pd.read_csv('model_a_safety_10k.csv')
    
    # Features & Target
    X = df_safety.drop('Label_Safety', axis=1)
    y = df_safety['Label_Safety']
    
    # Split
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    # Train (TinyML Optimized)
    clf_safety = RandomForestClassifier(n_estimators=20, max_depth=10, random_state=42)
    clf_safety.fit(X_train, y_train)
    
    # Evaluate
    y_pred = clf_safety.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    
    print(f"✓ Model A Accuracy: {acc:.4f}")
    print("  Classification Report:")
    print(classification_report(y_test, y_pred, target_names=['Normal', 'Warning', 'Critical']))
    print("  Confusion Matrix:")
    print(confusion_matrix(y_test, y_pred))
    
    # Save
    joblib.dump(clf_safety, 'models/model_a_safety.pkl')
    print("✓ Saved to models/model_a_safety.pkl")
    
except Exception as e:
    print(f"❌ Error training Model A: {e}")

# ============================================
# 2. TRAIN MODEL B - HEALTH (SOH Estimation)
# ============================================
print("\n[2/3] Training Model B - Health Classification...")

try:
    # Load Data
    df_health = pd.read_csv('model_b_health_10k.csv')
    
    # Features & Target
    X = df_health.drop('Label_Health', axis=1)
    y = df_health['Label_Health']
    
    # Split
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    # Train (TinyML Optimized)
    clf_health = RandomForestClassifier(n_estimators=20, max_depth=10, random_state=42)
    clf_health.fit(X_train, y_train)
    
    # Evaluate
    y_pred = clf_health.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    
    print(f"✓ Model B Accuracy: {acc:.4f}")
    print("  Classification Report:")
    print(classification_report(y_test, y_pred, target_names=['Good', 'Bad']))
    print("  Confusion Matrix:")
    print(confusion_matrix(y_test, y_pred))
    
    # Save
    joblib.dump(clf_health, 'models/model_b_health.pkl')
    print("✓ Saved to models/model_b_health.pkl")

except Exception as e:
    print(f"❌ Error training Model B: {e}")

# ============================================
# 3. TRAIN MODEL C - DRIVER (Indian Roads)
# ============================================
print("\n[3/3] Training Model C - Driver Profiling (Indian Context)...")

try:
    # Load Data
    df_driver = pd.read_csv('model_c_driver_10k.csv')
    
    # Features & Target
    X = df_driver.drop('Label_Driver', axis=1)
    y = df_driver['Label_Driver']
    
    # Split
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    # Train (TinyML Optimized)
    clf_driver = RandomForestClassifier(n_estimators=20, max_depth=10, random_state=42)
    clf_driver.fit(X_train, y_train)
    
    # Evaluate
    y_pred = clf_driver.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    
    print(f"✓ Model C Accuracy: {acc:.4f}")
    print("  Classification Report:")
    # Update target names for the new 3 classes
    print(classification_report(y_test, y_pred, target_names=['City', 'Highway', 'Emergency']))
    print("  Confusion Matrix:")
    print(confusion_matrix(y_test, y_pred))
    
    # Save
    joblib.dump(clf_driver, 'models/model_c_driver.pkl')
    print("✓ Saved to models/model_c_driver.pkl")

except Exception as e:
    print(f"❌ Error training Model C: {e}")

print("\n" + "="*80)
print("TRAINING COMPLETE")
print("="*80)
