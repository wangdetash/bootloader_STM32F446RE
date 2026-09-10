/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t bl_rx_buffer[256];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */
void bootlader_uart_read_data(void)
{
  uint8_t rcv_len;

  while (1)
  {
    memset(bl_rx_buffer, 0, sizeof(bl_rx_buffer));

    /* Read the number of bytes that follow the length byte. */
    if (HAL_UART_Receive(&huart3, bl_rx_buffer, 1, HAL_MAX_DELAY) != HAL_OK)
    {
      continue;
    }
    rcv_len = bl_rx_buffer[0];

    /* Keep the length at index 0 and receive the payload after it. */
    if (rcv_len > 0)
    {
      if (HAL_UART_Receive(&huart3, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY) != HAL_OK)
      {
        continue;
      }

      /* The second byte contains the command code. */
      switch (bl_rx_buffer[1])
      {
        case BL_GET_VER:
          /* TODO: Return the bootloader version. */
          break;

        case BL_GET_HELP:
          /* TODO: Return the supported command codes. */
          break;

        case BL_GET_CID:
          /* TODO: Return the chip ID. */
          break;

        case BL_GET_RDP_STATUS:
          /* TODO: Return the read protection status. */
          break;

        case BL_GO_TO_ADDR:
          /* TODO: Jump to the requested address. */
          break;

        case BL_FLASH_ERASE:
          /* TODO: Erase the requested flash sectors. */
          break;

        case BL_MEM_WRITE:
          /* TODO: Write the supplied data to memory. */
          break;

        case BL_EN_R_W_PROTECT:
          /* TODO: Enable read/write protection. */
          break;

        case BL_MEM_READ:
          /* TODO: Read the requested memory. */
          break;

        case BL_READ_SECTOR_STATUS:
          /* TODO: Return the sector protection status. */
          break;

        case BL_OTP_READ:
          /* TODO: Read the requested OTP data. */
          break;

        case BL_DIS_R_W_PROTECT:
          /* TODO: Disable read/write protection. */
          break;

        default:
          /* Unknown command: ignore this packet. */
          break;
      }
    }
  }
}

void bootlaoder_uart_jump_to_user_app(void)
{
	//function to hold the address of reset handler of the3 user app
	void (*app_reset_handler)(void);

	// configure MSP by reading the value from the base addrress of sector 2
	uint32_t msp_value =  *(volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS;

	//The function comes from CMSIS
	__set_MSP(msp_value);

	//Fetching the reset handler address of user application fro the location (FLASH_SECTOR2_BASE_ADDRESS + 4)
	uint32_t resethandler_address =  *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS);

	app_reset_handler = (void*) resethandler_address;

	app_reset_handler();
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char Selectedbootloader[] = "Bootloader mode\r\n";
char SelectedApplication[] = "Application mode\r\n";
char InWhileLoop[] = "InWhileLoop\r\n";
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED2);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /*check whether the button is pressed or not*/
  if(HAL_GPIO_ReadPin(KEY_BUTTON_GPIO_PORT,USER_BUTTON_PIN) == GPIO_PIN_RESET)
  {
	  HAL_UART_Transmit(&huart2,(uint8_t*)Selectedbootloader,sizeof(Selectedbootloader),HAL_MAX_DELAY);
	  bootlader_uart_read_data();
  }
  else
  {
	  HAL_UART_Transmit(&huart2,(uint8_t*)SelectedApplication,sizeof(SelectedApplication),HAL_MAX_DELAY);
	  bootlaoder_uart_jump_to_user_app();
  }
  while (1)
  {

	  HAL_UART_Transmit(&huart3,(uint8_t*)InWhileLoop,sizeof(InWhileLoop),HAL_MAX_DELAY);
	  uint32_t current_tick = HAL_GetTick();
	  while(HAL_GetTick() <= (current_tick + 1000));
    /* USER CODE END WHILE */



    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
