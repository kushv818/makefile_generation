#include "can.h"
#include "rev_match.h"
#include <stdint.h>

extern FDCAN_HandleTypeDef hfdcan2;
static volatile VehicleData_t vehicle_data = {0};
uint32_t raw;
uint8_t data_141[8];


static uint16_t bitsToUIntLe(const uint8_t *data, uint16_t bit_offset, uint16_t bit_length);

void CAN_App_Init(void) {
    FDCAN_FilterTypeDef sFilterConfig;

    sFilterConfig.IdType       = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex  = 0;
    sFilterConfig.FilterType   = FDCAN_FILTER_DUAL;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1    = 0xD4; // wheel speed
    sFilterConfig.FilterID2    = 0x140; // clutch and rpm
    // if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig) != HAL_OK) {
    //     Error_Handler();
    // }

    sFilterConfig.IdType       = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex  = 1;
    sFilterConfig.FilterType   = FDCAN_FILTER_DUAL;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1    = 0x141; // redundant rpm and redundant current gear
    sFilterConfig.FilterID2    = 0x361; // current gear
    // if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig) != HAL_OK) {
    //     Error_Handler();
    // }

    // if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
    //     Error_Handler();
    // }

    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        Error_Handler();
    }
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if (hfdcan->Instance == FDCAN2) {
        if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
            FDCAN_RxHeaderTypeDef rx_header;
            uint8_t rx_data[8];

            if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
                
                if (rx_header.Identifier == WHEELSPEED_CAN_ID) {
                    //grab 2 bytes, little endian, 3rd and 4th pair
                    vehicle_data.bl_speed = (rx_data[6] | (rx_data[7] << 8)) / 28;
                    vehicle_data.br_speed = (rx_data[4] | (rx_data[5] << 8)) / 28;

                    raw = rx_data[4] | (rx_data[5] << 8) ;
                } 
                else if (rx_header.Identifier == CLUTCH_RPM_CAN_ID) {
                    vehicle_data.clutch_pressed = (rx_data[1] & 0x80) == 0x80;
                    vehicle_data.engine_rpm = (uint32_t)bitsToUIntLe(rx_data, 16, 14);
                }
                else if (rx_header.Identifier == GEAR_CAN_ID) {
                    for (int i = 0; i < 8; i++) 
                        data_141[i] = rx_data[i];

                    // netural is , 1-6, an
                     vehicle_data.raw_gear = rx_data[6] & 0xF;
                     if (vehicle_data.raw_gear ==7)
                        vehicle_data.current_gear = NEUTRAL_GEAR;
                    else
                     vehicle_data.current_gear = vehicle_data.raw_gear;
                }
                else if (rx_header.Identifier == OTHER_GEAR_CAN_ID) {
                    // vehicle_data.current_gear = rx_data[0] & 0x7;
                }

                vehicle_data.last_update_tick = HAL_GetTick();
            }
        }
    }
}

bool CAN_GetLatestData(VehicleData_t *dest_data) {
    if (dest_data == NULL) return false;

    __disable_irq();
    *dest_data = vehicle_data;
    __enable_irq();

    return true;
}

uint16_t bitsToUIntLe(const uint8_t *data, uint16_t bit_offset, uint16_t bit_length) {
    uint32_t raw_val = 0;
    
    uint8_t start_byte = bit_offset / 8;
    uint8_t shift = bit_offset % 8;
    
    for (int i = 0; i < 4; i++) {
        raw_val |= ((uint32_t)data[start_byte + i]) << (i * 8);
    }
    
    raw_val >>= shift;
    uint32_t mask = (1UL << bit_length) - 1;
    return (uint16_t)(raw_val & mask);
}