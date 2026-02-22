# EV Guardian: TinyML-Powered EV Diagnostic System

**EV Guardian** is an advanced diagnostic and safety monitoring system for Electric Vehicles (EVs). It leverages **TinyML (Machine Learning on Edge)** to perform real-time analysis of vehicle data directly on an ESP32 microcontroller, providing proactive safety alerts, battery health assessments, and driver behavior insights.

## 🚀 Overview

The system operates using two primary hardware components communicating over a **CAN Bus**:
1.  **ESP32 Sender (Vehicle Simulator):** Simulates real-world EV operating conditions (City, Highway, Emergency, Faults) and transmits sensor data via MCP2515 CAN controller.
2.  **ESP32 Receiver (TinyML Guardian):** Receives CAN data, processes it through optimized Random Forest models, and outputs diagnostic results.

## 🏗️ Project Structure

-   **`ESP32_Sender/`**: contains the vehicle simulation firmware.
    -   `ESP32_1_Sender_Simulator.ino`: Main simulator logic with multiple driving modes.
    -   `sim_config.h`: Hardware and simulation parameters.
-   **`ESP32_Reciever/`**: contains the diagnostic engine.
    -   `ESP32_2_Receiver_TinyML_Guardian.ino`: Core receiver and inference engine.
    -   `model_a_safety.h`, `model_b_health.h`, `model_c_driver.h`: Pre-compiled TinyML models.
    -   `model_utils.h`: Helper functions for feature extraction and label mapping.
-   **`ML_models/`**: (Workspace for model training)
    -   `generate_data.py`: Script to generate synthetic EV dataset for training.
    -   `train_all_models.py`: Trains Random Forest classifiers using Scikit-Learn.
    -   `convert_to_tinyml.py`: Converts trained Python models into C++ headers using `micromlgen` or `eloquent_tinyml`.

## 🧠 Diagnostic Capabilities

The system runs three specialized TinyML models:

### 1. Safety Monitoring (`model_a_safety`)
Analyzes electrical and thermal parameters to detect critical failures:
-   **Normal Operating Conditions**
-   **Overheating (Battery/Inverter)**
-   **Isolation Faults**
-   **Thermal Runaway Risk (Gas Detection)**
-   **Voltage Imbalance**

### 2. Battery Health (SoH) (`model_b_health`)
Assesses the long-term state of the battery pack:
-   **State of Health (SOH) Estimation** (Healthy vs. Degrading)
-   **Internal Resistance Tracking**
-   **Cycle Stress Analysis**

### 3. Driver Behavior (`model_c_driver`)
Profiles driving styles and efficiency:
-   **Eco-Friendly Driving**
-   **Aggressive Driving** (Harsh braking/acceleration)
-   **Distensive/Passive Driving**

## 🔌 Hardware Requirements

-   2x **ESP32** Microcontrollers
-   2x **MCP2515 CAN Bus Modules**
-   CAN Bus wiring (High/Low)
-   (Optional) Display for real-time alerts

## 🛠️ Getting Started

1.  **Hardware Setup:** Connect the ESP32s to MCP2515 modules via SPI. Connect the two CAN modules together.
2.  **Sender:** Upload `ESP32_1_Sender_Simulator.ino` to the first ESP32.
3.  **Receiver:** Upload `ESP32_2_Receiver_TinyML_Guardian.ino` to the second ESP32.
4.  **Monitor:** Open the Serial Monitor (115200 baud) for the Receiver to see real-time AI diagnostics.

---
Developed as a proactive safety layer for modern Electric Vehicles.
