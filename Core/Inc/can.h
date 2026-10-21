#ifndef CAN_H
#define CAN_H


#include "main.h"
#include "rev_match.h"
// #include <stdbool.h>


#define WHEELSPEED_CAN_ID 0xD4
#define CLUTCH_RPM_CAN_ID 0x140
#define GEAR_CAN_ID 0x141
#define OTHER_GEAR_CAN_ID 0x361

typedef struct {
    uint32_t engine_rpm;
    float br_speed;
    float bl_speed;
    bool clutch_pressed; // true is yes, false is no
    uint8_t raw_gear;
    uint8_t current_gear;
    uint32_t last_update_tick;
} VehicleData_t;

void CAN_App_Init(void);
bool CAN_GetLatestData(VehicleData_t *dest_data);

#endif