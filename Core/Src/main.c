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
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PM */
#define D_UART (&huart3)
#define C_UART (&huart2)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t bl_rx_buffer[256];
const uint8_t supported_commands[] = {
  BL_GET_VER,
  BL_GET_HELP,
  BL_GET_CID,
  BL_GET_RDP_STATUS,
  BL_GO_TO_ADDR,
  BL_FLASH_ERASE,
  BL_MEM_WRITE,
  BL_EN_R_W_PROTECT,
  BL_MEM_READ,
  BL_READ_SECTOR_STATUS,
  BL_OTP_READ,
  BL_DIS_R_W_PROTECT
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

void printmsg(const char *message);
uint16_t get_mcu_chip_id(void);
uint8_t get_flashrdp_level(void);
uint8_t verify_address(uint32_t go_address);
void bootlader_uart_read_data(void);
void bootlaoder_uart_jump_to_user_app(void);
void bootloader_handle_getver_cmd(uint8_t *bl_rx_buffer);
void bootloader_handle_gethelp_cmd(uint8_t *bl_rx_buffer);
void bootloader_handle_getcid_cmd(uint8_t *bl_rx_buffer);
void bootloader_handle_getrdp_cmd(uint8_t *bl_rx_buffer);
void bootloader_handle_go_cmd(uint8_t *bl_rx_buffer);
void bootloader_handle_flash_erase_cmd(uint8_t *bl_rx_buffer);
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sectors);
void bootloader_handle_mem_write_cmd(uint8_t *bl_rx_buffer);
uint8_t execute_mem_write(uint8_t *buffer, uint32_t mem_address, uint32_t length);
void bootloader_handle_endis_rw_protect(uint8_t *bl_rx_buffer);
void bootloader_handle_mem_read(uint8_t *bl_rx_buffer);
void bootloader_handle_read_sector_status(uint8_t *bl_rx_buffer);
void bootloader_handle_read_otp(uint8_t *bl_rx_buffer);
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
	  printmsg(Selectedbootloader);
	  bootlader_uart_read_data();
  }
  else
  {
	  printmsg(SelectedApplication);
	  bootlaoder_uart_jump_to_user_app();
  }
  while (1)
  {

	  printmsg(InWhileLoop);
//	  uint32_t current_tick = HAL_GetTick();
//	  while(HAL_GetTick() <= (current_tick + 1000));
	  HAL_Delay(1000);
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
/**
  * @brief Send a null-terminated message through the debug UART.
  */
void printmsg(const char *message)
{
  HAL_UART_Transmit(D_UART, (const uint8_t *)message,
                    (uint16_t)strlen(message), HAL_MAX_DELAY);
}


/*
 * Provisional request: [length][command][arguments...][CRC32 LE].
 * length counts all bytes after itself, including the CRC.
 * Hardware CRC covers length, command and arguments (not the CRC), with
 * each byte zero-extended to one 32-bit word, MSB first, no final XOR.
 * len excludes the trailing host CRC bytes.
 */
uint8_t bootloader_verify_crc(uint8_t *p_data, uint8_t len, uint32_t crc_host)
{
  uint32_t uwCRCvalue = 0xFFU;

  if (p_data == NULL || len == 0U)
  {
    return VERIFY_CRC_FAILURE;
  }

  __HAL_CRC_DR_RESET(&hcrc);
  for (uint16_t i = 0U; i < len; ++i)
  {
    uint32_t data_word = p_data[i];
    uwCRCvalue = HAL_CRC_Accumulate(&hcrc, &data_word, 1U);
  }

  return (uwCRCvalue == crc_host) ? VERIFY_CRC_SUCCESS : VERIFY_CRC_FAILURE;
}

/* Validate framing before extracting the host CRC or narrowing the data length. */
static uint8_t bootloader_verify_packet(uint8_t *packet)
{
  if (packet == NULL)
  {
    return VERIFY_CRC_FAILURE;
  }

  uint16_t packet_len = (uint16_t)packet[0] + 1U;
  if (packet_len < BL_MIN_PACKET_SIZE || packet_len > sizeof(bl_rx_buffer))
  {
    return VERIFY_CRC_FAILURE;
  }

  uint16_t data_len = packet_len - BL_CRC_SIZE;
  uint32_t host_crc = (uint32_t)packet[data_len] |
                      ((uint32_t)packet[data_len + 1U] << 8U) |
                      ((uint32_t)packet[data_len + 2U] << 16U) |
                      ((uint32_t)packet[data_len + 3U] << 24U);
  /* A maximum 256-byte packet contains only 252 bytes covered by the CRC. */
  return bootloader_verify_crc(packet, (uint8_t)data_len, host_crc);
}

HAL_StatusTypeDef bootloader_uart_write_data(const uint8_t *data, uint16_t length)
{
  if (data == NULL || length == 0U)
  {
    return HAL_ERROR;
  }
  return HAL_UART_Transmit(C_UART, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef bootloader_send_ack(uint8_t reply_len)
{
  const uint8_t ack[] = {BL_ACK, reply_len};
  return bootloader_uart_write_data(ack, sizeof(ack));
}

HAL_StatusTypeDef bootloader_send_nack(void)
{
  const uint8_t nack = BL_NACK;
  return bootloader_uart_write_data(&nack, sizeof(nack));
}

uint8_t get_bootloader_version(void)
{
  return BL_VERSION;
}

uint16_t get_mcu_chip_id(void)
{
  return (uint16_t)(DBGMCU->IDCODE & DBGMCU_IDCODE_DEV_ID);
}

uint8_t get_flashrdp_level(void)
{
  uint8_t rdp_status = (uint8_t)((FLASH->OPTCR >> 8U) & 0xFFU);
  return rdp_status;
}

uint8_t verify_address(uint32_t go_address)
{
  if ((go_address >= FLASH_BASE) && (go_address <= FLASH_END))
  {
    return ADD_VALID;
  }

  if ((go_address >= SYSTEM_MEMORY_BASE_ADDRESS) &&
      (go_address <= SYSTEM_MEMORY_END_ADDRESS))
  {
    return ADD_VALID;
  }

  if ((go_address >= SRAM1_BASE) && (go_address < SRAM1_END))
  {
    return ADD_VALID;
  }

  if ((go_address >= SRAM2_BASE) && (go_address < SRAM2_END))
  {
    return ADD_VALID;
  }

  /* Check backup SRAM before the peripheral region, which contains it. */
  if ((go_address >= BKPSRAM_BASE) && (go_address < BKPSRAM_END))
  {
    return ADD_VALID;
  }

  if ((go_address >= PERIPH_BASE) &&
      (go_address <= PERIPHERAL_MEMORY_END_ADDRESS))
  {
    return ADD_INVALID;
  }

  if ((go_address >= EXTERNAL_MEMORY_BASE_ADDRESS) &&
      (go_address <= EXTERNAL_MEMORY_END_ADDRESS))
  {
    return ADD_VALID;
  }

  return ADD_INVALID;
}

void bootloader_handle_getver_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("GET_VER: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* GET_VER has no command arguments. */
  if ((uint16_t)packet[0] + 1U != BL_MIN_PACKET_SIZE)
  {
    printmsg("GET_VER: unexpected arguments\r\n");
    bootloader_send_nack();
    return;
  }

  if (bootloader_send_ack(1U) != HAL_OK)
  {
    return;
  }
  uint8_t bl_version = get_bootloader_version();
  char message[40];
  snprintf(message, sizeof(message), "Bootloader version: 0x%02X\r\n",
           (unsigned int)bl_version);
  printmsg(message);
  bootloader_uart_write_data(&bl_version, sizeof(bl_version));
}

void bootloader_handle_gethelp_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("GET_HELP: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* GET_HELP has no command arguments. */
  if ((uint16_t)packet[0] + 1U != BL_MIN_PACKET_SIZE)
  {
    printmsg("GET_HELP: unexpected arguments\r\n");
    bootloader_send_nack();
    return;
  }

  uint8_t command_count = (uint8_t)sizeof(supported_commands);
  if (bootloader_send_ack(command_count) != HAL_OK)
  {
    return;
  }
  printmsg("GET_HELP: sending supported commands\r\n");
  bootloader_uart_write_data(supported_commands, command_count);
}

void bootloader_handle_getcid_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("GET_CID: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* GET_CID has no command arguments. */
  if ((uint16_t)packet[0] + 1U != BL_MIN_PACKET_SIZE)
  {
    printmsg("GET_CID: unexpected arguments\r\n");
    bootloader_send_nack();
    return;
  }

  uint16_t bl_cid_num = get_mcu_chip_id();
  if (bootloader_send_ack((uint8_t)sizeof(bl_cid_num)) != HAL_OK)
  {
    return;
  }
  printmsg("GET_CID: sending MCU chip ID\r\n");
  bootloader_uart_write_data((const uint8_t *)&bl_cid_num,
                             (uint16_t)sizeof(bl_cid_num));
}

void bootloader_handle_getrdp_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("GET_RDP_STATUS: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* GET_RDP_STATUS has no command arguments. */
  if ((uint16_t)packet[0] + 1U != BL_MIN_PACKET_SIZE)
  {
    printmsg("GET_RDP_STATUS: unexpected arguments\r\n");
    bootloader_send_nack();
    return;
  }

  uint8_t rdp_status = get_flashrdp_level();
  if (bootloader_send_ack((uint8_t)sizeof(rdp_status)) != HAL_OK)
  {
    return;
  }
  printmsg("GET_RDP_STATUS: sending RDP option byte\r\n");
  bootloader_uart_write_data(&rdp_status, (uint16_t)sizeof(rdp_status));
}

void bootloader_handle_go_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("GO_TO_ADDR: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }

  /* GO_TO_ADDR carries exactly one 32-bit address argument. */
  if ((uint16_t)packet[0] + 1U !=
      (BL_MIN_PACKET_SIZE + (uint16_t)sizeof(uint32_t)))
  {
    printmsg("GO_TO_ADDR: invalid packet length\r\n");
    bootloader_send_nack();
    return;
  }

  if (bootloader_send_ack(1U) != HAL_OK)
  {
    return;
  }

  uint32_t go_address;
  memcpy(&go_address, &packet[2], sizeof(go_address));

  char message[64];
  snprintf(message, sizeof(message), "GO_TO_ADDR: address 0x%08lX\r\n",
           (unsigned long)go_address);
  printmsg(message);

  uint8_t address_status = verify_address(go_address);
  if (bootloader_uart_write_data(&address_status, sizeof(address_status)) != HAL_OK)
  {
    return;
  }

  if (address_status == ADD_VALID)
  {
    printmsg("GO_TO_ADDR: address is valid\r\n");

    /* Cortex-M function pointers must have the Thumb-state bit set. */
    go_address += 1U;
    void (*go_to_address)(void) = (void (*)(void))go_address;

    snprintf(message, sizeof(message), "GO_TO_ADDR: going to 0x%08lX\r\n",
             (unsigned long)go_address);
    printmsg(message);
    go_to_address();
  }
  else
  {
    printmsg("GO_TO_ADDR: address is invalid\r\n");
  }
}

void bootloader_handle_flash_erase_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("FLASH_ERASE: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }

  /* FLASH_ERASE carries a sector number and the number of sectors to erase. */
  if ((uint16_t)packet[0] + 1U !=
      (BL_MIN_PACKET_SIZE + (2U * (uint16_t)sizeof(uint8_t))))
  {
    printmsg("FLASH_ERASE: invalid packet length\r\n");
    bootloader_send_nack();
    return;
  }

  uint8_t sector_number = packet[2];
  uint8_t number_of_sectors = packet[3];

  if (bootloader_send_ack(1U) != HAL_OK)
  {
    return;
  }

  BSP_LED_On(LED2);
  uint8_t erase_status = execute_flash_erase(sector_number, number_of_sectors);
  BSP_LED_Off(LED2);

  bootloader_uart_write_data(&erase_status, sizeof(erase_status));
}

uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sectors)
{
  FLASH_EraseInitTypeDef erase_config = {0};
  uint32_t sector_error = 0xFFFFFFFFU;
  HAL_StatusTypeDef erase_status;

  if ((number_of_sectors < 1U) || (number_of_sectors > 7U))
  {
    return (uint8_t)HAL_ERROR;
  }

  if (sector_number == 0xFFU)
  {
    erase_config.TypeErase = FLASH_TYPEERASE_MASSERASE;
  }
  else
  {
    if ((sector_number > FLASH_SECTOR_7) ||
        (((uint16_t)sector_number + (uint16_t)number_of_sectors) > 8U))
    {
      return (uint8_t)HAL_ERROR;
    }

    erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_config.Sector = sector_number;
    erase_config.NbSectors = number_of_sectors;
  }

  erase_config.Banks = FLASH_BANK_1;
  erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  erase_status = HAL_FLASH_Unlock();
  if (erase_status == HAL_OK)
  {
    erase_status = HAL_FLASHEx_Erase(&erase_config, &sector_error);
    if ((erase_status == HAL_OK) && (sector_error != 0xFFFFFFFFU))
    {
      erase_status = HAL_ERROR;
    }
  }

  if (HAL_FLASH_Lock() != HAL_OK)
  {
    erase_status = HAL_ERROR;
  }

  return (uint8_t)erase_status;
}

void bootloader_handle_mem_write_cmd(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("MEM_WRITE: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }

  /* MEM_WRITE carries a 32-bit address, a one-byte payload length and data. */
  if ((uint16_t)packet[0] + 1U <
      (BL_MIN_PACKET_SIZE + (uint16_t)sizeof(uint32_t) +
       (uint16_t)sizeof(uint8_t)))
  {
    printmsg("MEM_WRITE: invalid packet length\r\n");
    bootloader_send_nack();
    return;
  }

  uint8_t payload_length = packet[6];
  uint16_t expected_packet_length = BL_MIN_PACKET_SIZE +
                                    (uint16_t)sizeof(uint32_t) +
                                    (uint16_t)sizeof(payload_length) +
                                    (uint16_t)payload_length;
  if (((uint16_t)packet[0] + 1U != expected_packet_length) ||
      (payload_length == 0U))
  {
    printmsg("MEM_WRITE: invalid packet length\r\n");
    bootloader_send_nack();
    return;
  }

  if (bootloader_send_ack(1U) != HAL_OK)
  {
    return;
  }

  uint32_t mem_address;
  memcpy(&mem_address, &packet[2], sizeof(mem_address));

  uint8_t write_status = ADD_INVALID;
  if (verify_address(mem_address) == ADD_VALID)
  {
    BSP_LED_On(LED2);
    write_status = execute_mem_write(&packet[7], mem_address, payload_length);
    BSP_LED_Off(LED2);
  }

  bootloader_uart_write_data(&write_status, sizeof(write_status));
}

uint8_t execute_mem_write(uint8_t *buffer, uint32_t mem_address, uint32_t length)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* HAL_FLASH_Program is valid only when the complete write lies in flash. */
  if ((buffer == NULL) || (length == 0U) ||
      (mem_address < FLASH_BASE) || (mem_address > FLASH_END) ||
      ((length - 1U) > (FLASH_END - mem_address)))
  {
    return (uint8_t)HAL_ERROR;
  }

  status = HAL_FLASH_Unlock();
  if (status == HAL_OK)
  {
    for (uint32_t index = 0U; index < length; ++index)
    {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                                 mem_address + index,
                                 buffer[index]);
      if (status != HAL_OK)
      {
        break;
      }
    }
  }

  if (HAL_FLASH_Lock() != HAL_OK)
  {
    status = HAL_ERROR;
  }

  return (uint8_t)status;
}

void bootloader_handle_endis_rw_protect(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("EN/DIS_R_W_PROTECT: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* ACK confirms receipt/CRC only; command execution remains a TODO. */
  if (bootloader_send_ack(0U) != HAL_OK)
  {
    return;
  }
  printmsg("EN/DIS_R_W_PROTECT: handler not implemented\r\n");
}

void bootloader_handle_mem_read(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("MEM_READ: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* ACK confirms receipt/CRC only; command execution remains a TODO. */
  if (bootloader_send_ack(0U) != HAL_OK)
  {
    return;
  }
  printmsg("MEM_READ: handler not implemented\r\n");
}

void bootloader_handle_read_sector_status(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("READ_SECTOR_STATUS: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* ACK confirms receipt/CRC only; command execution remains a TODO. */
  if (bootloader_send_ack(0U) != HAL_OK)
  {
    return;
  }
  printmsg("READ_SECTOR_STATUS: handler not implemented\r\n");
}

void bootloader_handle_read_otp(uint8_t *packet)
{
  if (bootloader_verify_packet(packet) != VERIFY_CRC_SUCCESS)
  {
    printmsg("OTP_READ: invalid packet or CRC\r\n");
    bootloader_send_nack();
    return;
  }
  /* ACK confirms receipt/CRC only; command execution remains a TODO. */
  if (bootloader_send_ack(0U) != HAL_OK)
  {
    return;
  }
  printmsg("OTP_READ: handler not implemented\r\n");
}

void bootlader_uart_read_data(void)
{
  uint8_t rcv_len;

  while (1)
  {
    memset(bl_rx_buffer, 0, sizeof(bl_rx_buffer));

    /* Read the number of bytes that follow the length byte. */
    if (HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY) != HAL_OK)
    {
      continue;
    }
    rcv_len = bl_rx_buffer[0];

    /* Keep the length at index 0 and receive the payload after it. */
    if (rcv_len > 0)
    {
      if (HAL_UART_Receive(C_UART, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY) != HAL_OK)
      {
        continue;
      }

      /* The second byte contains the command code. */
      switch (bl_rx_buffer[1])
      {
        case BL_GET_VER:
          bootloader_handle_getver_cmd(bl_rx_buffer);
          break;

        case BL_GET_HELP:
          bootloader_handle_gethelp_cmd(bl_rx_buffer);
          break;

        case BL_GET_CID:
          bootloader_handle_getcid_cmd(bl_rx_buffer);
          break;

        case BL_GET_RDP_STATUS:
          bootloader_handle_getrdp_cmd(bl_rx_buffer);
          break;

        case BL_GO_TO_ADDR:
          bootloader_handle_go_cmd(bl_rx_buffer);
          break;

        case BL_FLASH_ERASE:
          bootloader_handle_flash_erase_cmd(bl_rx_buffer);
          break;

        case BL_MEM_WRITE:
          bootloader_handle_mem_write_cmd(bl_rx_buffer);
          break;

        case BL_EN_R_W_PROTECT:
          bootloader_handle_endis_rw_protect(bl_rx_buffer);
          break;

        case BL_MEM_READ:
          bootloader_handle_mem_read(bl_rx_buffer);
          break;

        case BL_READ_SECTOR_STATUS:
          bootloader_handle_read_sector_status(bl_rx_buffer);
          break;

        case BL_OTP_READ:
          bootloader_handle_read_otp(bl_rx_buffer);
          break;

        case BL_DIS_R_W_PROTECT:
          bootloader_handle_endis_rw_protect(bl_rx_buffer);
          break;

        default:
          printmsg("Invalid command recieved from the host\r\n");
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
	uint32_t resethandler_address =  *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4);

	app_reset_handler = (void*) resethandler_address;

	app_reset_handler();
}

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
