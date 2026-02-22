#ifndef SIM_CONFIG_H
#define SIM_CONFIG_H

// CAN BUS CONFIGURATION (MCP2515)
#define CAN_CS_PIN  5    // Chip Select
#define CAN_INT_PIN 4    // Interrupt (Optional)

// CAN IDs (Extended or Standard)
#define CAN_ID_SAFETY_1  0x100
#define CAN_ID_SAFETY_2  0x101
#define CAN_ID_SAFETY_3  0x102
#define CAN_ID_HEALTH_1  0x200
#define CAN_ID_HEALTH_2  0x201
#define CAN_ID_DRIVER_1  0x300
#define CAN_ID_DRIVER_2  0x301

// Simulation Timing
#define UPDATE_RATE_MS 100 // 10Hz Update Rate

#endif
