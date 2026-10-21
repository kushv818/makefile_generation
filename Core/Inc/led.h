#ifndef LED_H
#define LED_H

#include "main.h"


// LED strip properties
#define NUM_LEDS 180
#define BITS_PER_LED 24    // 24 bits for RGB
#define DMA_BUFF_SIZE ((NUM_LEDS * BITS_PER_LED) + 50) // Data bits + Reset padding

// LED strip RGB data
extern uint8_t led_data[NUM_LEDS][3];

// PWM duty cycle values
// Timer Clock = 80MHz, ARR = 99 (Period = 1.25us for 800kHz)
#define PWM_BIT_1 50    // 50/99 -> 50% duty cycle
#define PWM_BIT_0 25    // 25/99 -> 25% duty cycle


void SK6812_SetLED(uint16_t index, uint8_t r, uint8_t g, uint8_t b);

void SK6812_Clear_All_LED();

void SK6812_Update(void);

#endif 