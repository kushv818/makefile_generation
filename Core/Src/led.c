#include "led.h"

extern TIM_HandleTypeDef htim1;
//extern DMA_HandleTypeDef hdma_tim1_ch1;

// DMA Buffer holds the PWM duty cycles for every bit
// this one frame controls entire led strip
uint16_t dma_led_bit_buffer[DMA_BUFF_SIZE];
uint8_t led_data[NUM_LEDS][3]; // Stores raw [G, R, B] per LED


/**
  * @brief  Sets the RGB color of an individual LED in the data array
    used on CAN callback or whenever led strip is to be updated
  */
void SK6812_SetLED(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < NUM_LEDS) {
        led_data[index][0] = g;
        led_data[index][1] = r;
        led_data[index][2] = b;
    }
}

void SK6812_Clear_All_LED() {
    for (int i =0; i < NUM_LEDS; i++) {
        led_data[i][0] = 0;
        led_data[i][1] = 0;
        led_data[i][2] = 0;
    }
}

/**
  * @brief  Translates raw RGB data into PWM duty cycles and sends via DMA
  */
void SK6812_Update(void) {
    uint32_t indx = 0;
    uint32_t color;

    for (int i = 0; i < NUM_LEDS; i++) {
        // Combine G, R, B into a 24-bit frame
        color = ((led_data[i][0] << 16) | 
                 (led_data[i][1] << 8) | 
                 (led_data[i][2]));

        // Parse 24 bits and add duty cycle to DMA buffer
        for (int i_bit = 23; i_bit >= 0; i_bit--) {
            if (color & (1 << i_bit)) {
                dma_led_bit_buffer[indx] = PWM_BIT_1;
            } else {
                dma_led_bit_buffer[indx] = PWM_BIT_0;
            }
            indx++;
        }
    }

    // Pad the end of the buffer with zeros to create the >80µs Reset Pulse
    for (int i = 0; i < 50; i++) {
        dma_led_bit_buffer[indx] = 0;
        indx++;
    }

    // Transmit the buffer using HAL DMA PWM
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)dma_led_bit_buffer, indx);
}


/**
  * @brief  Optional: DMA Transfer Complete Callback to stop PWM after one pass
  */
 void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
     if (htim->Instance == TIM1) {
         HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
     }
}
