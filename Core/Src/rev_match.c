#include "rev_match.h"
#include "led.h"

// calculation constants
#define RATIO_FINAL 4.1f
#define TIRE_CIRCUMFERENCE_INCHES 78.0f
#define MPH_TO_INCH_MIN 1056

// data arrays
uint16_t target_rpms[6] ={0};
uint8_t gear_led_indices[6] = {0};
const float gear_ratios[NUM_GEARS] = {3.626f, 2.188f, 1.541f, 1.213f, 1.0f, 0.767f};

//convert rpms to led indices
static uint16_t compress_rpm_to_led_range(uint16_t rpm_low, uint16_t rpm_high, uint8_t led_low, uint8_t led_high, uint16_t current_rpm);

// future implementation for redundancy checks/wheel slippage/error handling
static uint16_t calculate_current_rpm();
static uint8_t calculate_current_wheelspeed();

// calculate target rpm per gear given current rpm
void update_target_rpms(){
    for (int i = 0; i < NUM_GEARS; i++){
        uint16_t target = CALCULATE_TARGET_RPM(
                            current_speed, 
                            gear_ratios[i], 
                            RATIO_FINAL, 
                            TIRE_CIRCUMFERENCE_INCHES);
        target_rpms[i] = target;
    }
}

// convert target rpms into led indices
void calculate_led_indicator_positions(void) {
    for (int i = 0; i < NUM_GEARS; i++) {

        /*

        dynamic range shifting needs work, if rpms fall too much while clutch pressed
        only small window of rpms will be visible even if we want to rev to 3k+

        if(current_rpm < 2000)
            gear_led_indices[i] = compress_rpm_to_led_range(
            0, 
            2000,
            0,
            NUM_LEDS,
            target_rpms[i]);

            */
        
            gear_led_indices[i] = compress_rpm_to_led_range(
            0,
            9000,
            0,
            NUM_LEDS,
            target_rpms[i]);
    }
}

// takes leds indices and updates led buffer
void render_gear_indicators(void) {
    for (int i = 0; i < NUM_GEARS; i++) {
        uint8_t position = gear_led_indices[i];

        if (i == current_gear)
            SK6812_SetLED(position, 0, 0, 200); // Blue (Current)
        else if (i < current_gear) 
            SK6812_SetLED(position, 200, 0, 0); // Red (Downshift)
        else 
            SK6812_SetLED(position, 0, 200, 0); // Green (Upshift)
    }
    
    //if clutch is pressed, rpm indicator is free to move wherever and is not tied to a gear
    // if(clutch){
        uint8_t free_rpm_index = compress_rpm_to_led_range(
            0,
            9000,
            0,
            NUM_LEDS,
            current_rpm);
        SK6812_SetLED(free_rpm_index, 150, 150, 0); // Yellow (Free)
    //}
}

void rev_match_update(){
    SK6812_Clear_All_LED();
    update_target_rpms();
    calculate_led_indicator_positions();
    render_gear_indicators();
    SK6812_Update();
}

//takes an rpm range and maps it to a range of led strip indices
static uint16_t compress_rpm_to_led_range(uint16_t rpm_low, uint16_t rpm_high, uint8_t led_low, uint8_t led_high, uint16_t current_rpm) {
    if (current_rpm <= rpm_low) return led_low;
    if (current_rpm >= rpm_high) return led_high;
    return (uint16_t)((uint32_t)(current_rpm - rpm_low) * (led_high - led_low) / (rpm_high - rpm_low) + led_low);
}


// just for compiler, unfinished functions
static uint16_t calculate_current_rpm() {return current_rpm;}
static uint8_t calculate_current_wheelspeed() {return current_speed;}