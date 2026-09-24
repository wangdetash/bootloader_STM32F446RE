/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "stm32f4xx_nucleo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void bootlader_uart_read_data(void);
void bootlaoder_uart_jump_to_user_app(void);
void printmsg(const char *message);
uint8_t bootloader_verify_crc(uint8_t *p_data, uint8_t len, uint32_t crc_host);
HAL_StatusTypeDef bootloader_send_ack(uint8_t reply_len);
HAL_StatusTypeDef bootloader_send_nack(void);
uint8_t get_bootloader_version(void);
HAL_StatusTypeDef bootloader_uart_write_data(const uint8_t *data, uint16_t length);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define FLASH_SECTOR2_BASE_ADDRESS   0x08008000U

/* Provisional host protocol; change these together with the host software. */
#define BL_ACK                  0xA5U
#define BL_NACK                 0x7FU
#define BL_VERSION             0x10U
#define BL_CRC_SIZE             4U
#define BL_MIN_PACKET_SIZE      6U
#define VERIFY_CRC_SUCCESS      0U
#define VERIFY_CRC_FAILURE      1U

/* Command codes from Bootloader Commands.pdf. */
#define BL_GET_VER              0x51U
#define BL_GET_HELP             0x52U
#define BL_GET_CID              0x53U
#define BL_GET_RDP_STATUS       0x54U
#define BL_GO_TO_ADDR           0x55U
#define BL_FLASH_ERASE          0x56U
#define BL_MEM_WRITE            0x57U
#define BL_EN_R_W_PROTECT       0x58U
#define BL_MEM_READ             0x59U
#define BL_READ_SECTOR_STATUS   0x5AU
#define BL_OTP_READ             0x5BU
#define BL_DIS_R_W_PROTECT      0x5CU
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
