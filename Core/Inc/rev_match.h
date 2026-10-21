#ifndef REV_MATCH_H
#define REV_MATCH_H

#include "main.h"
#include "led.h"

// CANbus data
extern uint8_t current_gear; //where 0 is first gear
extern uint32_t current_speed; // in mph
extern uint16_t current_rpm;
extern bool clutch; //true indicates pedal pressed, false indicates pedal not pressed

typedef enum{
    NEUTRAL_GEAR = 0,
    FIRST_GEAR,
    SECOND_GEAR,
    THIRD_GEAR,
    FOURTH_GEAR,
    FIFTH_GEAR,
    SIXTH_GEAR
} gear;

// formula constants
#define NUM_GEARS 6

extern const float gear_ratios[NUM_GEARS];
extern uint16_t target_rpms[6];



#define CALCULATE_TARGET_RPM(mph, ratio_gear, ratio_final, circ_inches)\
    (mph * MPH_TO_INCH_MIN * ratio_gear * ratio_final) / (circ_inches)

// helper functions, ended up being all static
//uint16_t compress_rpm_to_led_range(uint16_t rpm_low, uint16_t rpm_high, uint8_t led_low, uint8_t led_high, uint16_t current rpm);
//uint16_t calculate_current_rpm();
//uint8_t calculate current wheelspeed();

//driver functions
void calculate_led_indicator_positions();
void update_target_rpms();
void render_gear_indicators();
void rev_match_update();

#endif 