/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "userFunctions.h"
//For first attempt
//#include "ws2812.h"
//#include "stripEffects.h"
//For LED second attempt
#include "WS2812_Lib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
//TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim10;
//DMA_HandleTypeDef hdma_tim1_ch1;

UART_HandleTypeDef huart3;

/* Definitions for temperatureT */
osThreadId_t temperatureTHandle;
uint32_t temperatureTBuffer[ 128 ];
osStaticThreadDef_t temperatureTControlBlock;
const osThreadAttr_t temperatureT_attributes = {
  .name = "temperatureT",
  .stack_mem = &temperatureTBuffer[0],
  .stack_size = sizeof(temperatureTBuffer),
  .cb_mem = &temperatureTControlBlock,
  .cb_size = sizeof(temperatureTControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for updateLeds */
osThreadId_t updateLedsHandle;
const osThreadAttr_t updateLeds_attributes = {
  .name = "updateLeds",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for blinkLED */
osThreadId_t blinkLEDHandle;
uint32_t blinkLEDBuffer[ 128 ];
osStaticThreadDef_t blinkLEDControlBlock;
const osThreadAttr_t blinkLED_attributes = {
  .name = "blinkLED",
  .stack_mem = &blinkLEDBuffer[0],
  .stack_size = sizeof(blinkLEDBuffer),
  .cb_mem = &blinkLEDControlBlock,
  .cb_size = sizeof(blinkLEDControlBlock),
  .priority = (osPriority_t) osPriorityBelowNormal7,
};
/* USER CODE BEGIN PV */

/*
 * \
 */


/*
 * User defined Threads
 */

/* Definitions for Main Task */	//TODO
osThreadId_t taskManagerHandle;
uint32_t taskManagerBuffer[ 128 ];
osStaticThreadDef_t taskManagerControlBlock;
const osThreadAttr_t taskManager_attributes = {
  .name = "taskManager",
  .stack_mem = &taskManagerBuffer[0],
  .stack_size = sizeof(taskManagerBuffer),
  .cb_mem = &taskManagerControlBlock,
  .cb_size = sizeof(taskManagerControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};

/* Definitions for userSignal3 */
osThreadId_t userSignal3Handle;
uint32_t userSignal3Buffer[ 128 ];
osStaticThreadDef_t userSignal3ControlBlock;
const osThreadAttr_t userSignal3_attributes = {
  .name = "userSignal3",
  .stack_mem = &userSignal3Buffer[0],
  .stack_size = sizeof(userSignal3Buffer),
  .cb_mem = &userSignal3ControlBlock,
  .cb_size = sizeof(userSignal3ControlBlock),
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* This thread sets the LED effect */
osThreadId_t ledEffectHandle;
uint32_t ledEffectBuffer[ 128 ];
osStaticThreadDef_t ledEffectControlBlock;
const osThreadAttr_t ledEffect_attributes = {
  .name = "ledEffect",
  .stack_mem = &ledEffectBuffer[0],
  .stack_size = sizeof(ledEffectBuffer),
  .cb_mem = &ledEffectControlBlock,
  .cb_size = sizeof(ledEffectControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};
/* This thread sets the LED effect */
osThreadId_t ledAttempt2Handle;
uint32_t ledAttempt2Buffer[ 128 ];
osStaticThreadDef_t ledAttempt2ControlBlock;
const osThreadAttr_t ledAttempt2_attributes = {
  .name = "ledAttempt2",
  .stack_mem = &ledAttempt2Buffer[0],
  .stack_size = sizeof(ledAttempt2Buffer),
  .cb_mem = &ledAttempt2ControlBlock,
  .cb_size = sizeof(ledAttempt2ControlBlock),
  .priority = (osPriority_t) osPriorityAboveNormal1,
};

//functionPrototype
void ledEffect(void *argument);
void userSignal3(void *argument);
void ledAttempt2(void *argument);




uint8_t count,Temp_byte1,Temp_byte2,errorCounter;

int16_t temperature;
float temp_float;

//These variables should go away
uint32_t WSdata1, WSdata2;
uint32_t WSdata[] = {0x0000ff,0x00ff00, 0xff0000,0x0000ff,0x00ff00, 0xff0000,0xffff00,0x00ffff,0xff00ff};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
void runTemperature(void *argument);
void startLeds(void *argument);
void blinkingLED(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len){
	int i=0;
	for (i=0; i<len; i++){
		ITM_SendChar((*ptr++));
	}
	return len;
}
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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_TIM6_Init();
  MX_TIM10_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  printf("initializing...");
  //Temperature initialization code
  HAL_TIM_Base_Start(&htim6);

  setPinAsInput();
  HAL_Delay(500);
  count = 0;
  errorCounter = 0;
  //LED initialization code
  HAL_TIM_Base_Start(&htim10);
  WSdata1 = 0x00FF0000;
  WSdata2 = 0x0000FF00;	//TODO review whether this can be deleted


  initTimerHandlers(&htim6,&htim10,&htim1);
  //HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  //htim2.Instance->CCR1 = 50;
  //uncomment for tests in the channel
  //HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  //htim1.Instance->CCR1 = 75;

  //This is for LED first attempt
  //HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *) LEDbuffer,LED_BUFFER_SIZE);
  //htim1.Instance->CCR1 = 0;	//The PWM starts with Duty Cycle 0 //TODO this should be included in the same init function once it is in the library
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of temperatureT */
  //temperatureTHandle = osThreadNew(runTemperature, NULL, &temperatureT_attributes);

  /* creation of updateLeds */
  //updateLedsHandle = osThreadNew(startLeds, NULL, &updateLeds_attributes);

  /* creation of blinkLED */
  //blinkLEDHandle = osThreadNew(blinkingLED, NULL, &blinkLED_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  //Thread for LED effect of first Library attempt
  //userSignal3Handle = osThreadNew(userSignal3, NULL, &userSignal3_attributes);	//TODO NULL should be the PWM timer
  //ledEffectHandle = osThreadNew(ledEffect, NULL, &ledEffect_attributes);
  //Superloop for LED effect of second Library attempt
  ledAttempt2Handle = osThreadNew(ledAttempt2, NULL, &ledAttempt2_attributes);

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
 
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */
	//This is for first attempt
	//fillBufferBlack();	//TODO This should be included in the final version of this init function that should be in the library

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  //htim1.Init.RepetitionCounter = 255;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 10-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xffff-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 1-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 0xffff - 1;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(userSignal_GPIO_Port, userSignal_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, userSignal2_Pin|userSignal3_Pin|temp1wire_Pin|WSLED_Pin 
                          |USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : userSignal_Pin */
  GPIO_InitStruct.Pin = userSignal_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(userSignal_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : userSignal2_Pin temp1wire_Pin USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = userSignal2_Pin|temp1wire_Pin|USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : userSignal3_Pin WSLED_Pin */
  GPIO_InitStruct.Pin = userSignal3_Pin|WSLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
//TODO this definition should be in a library
/*
 * @brief	This is the call function for the second attempt to control the WS2812
 */

void ledAttempt2(void *argument){
	printf("led attempt2 function called\n");

	  //This uses the WS2812 instead of WS2812b
	  uint8_t inc =0;
	  WS2812_HSV_t hsv_color;

	  hsv_color.v = 255;
	  hsv_color.s = 255;

	while (1) {
		WS2812_Shift_Right(0);
		WS2812_One_HSV(0, hsv_color, 1);

		hsv_color.h=(hsv_color.h > 339)?0: hsv_color.h + 20;

		osDelay(150);
	}

}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_runTemperature */
/**
  * @brief  Function implementing the temperatureT thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_runTemperature */
void runTemperature(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {

	  uint8_t Temp_byte1,Temp_byte2;
	  int16_t temp_intArray[10];
	  float temp_floatArray[10];
	  char numberString[7];
	  printf("Temperature Task Loop\n");

	  printf("Temperature will fnish\n");
	  osThreadTerminate(temperatureTHandle);

	  if(startOneWire() == -1)
	  	  errorCounter++;
	  //userDelayUs(1, usTimerHandler);
	  osDelay(1);

	  writeOneWire (0xCC);  // skip ROM
	  writeOneWire (0x44);  // convert t
	  osDelay(760);

	  if(startOneWire() == -1)
	  	  errorCounter++;
	  osDelay(1);
	  writeOneWire (0xCC);  // skip ROM
	  writeOneWire (0xBE);  // Read Scratch-pad

	  Temp_byte1 = readOneWire();
	  Temp_byte2 = readOneWire();

	  temperature = (Temp_byte2<<8) | Temp_byte1;
	  temp_float = (float)temperature/16;
	  temp_intArray[count] = temperature;
	  temp_floatArray[count] = temp_float;
	  if (float2string(temp_float, numberString)){
		  //HAL_UART_Transmit(&huart3, numberString, 7, 500);
		  printf("%s \n",numberString);//("T: %3.2f\n",temp_float);
	  }
	  //printf("T: %d\n", temp_intArray[count]);//("T: %3.2f\n",temp_float);
	  count++;
	  if (count > 20)
		  count = 0;

	  if (errorCounter > 10)
		  __NOP();

	  //if (float2string(temp_float, numberString))
		  //HAL_UART_Transmit(&huart3, numberString, 7, 500);

	 // else
		  //HAL_UART_Transmit(&huart3, &"nope\n", 7, 500);
	  //printf("Temperature Task Loop End\n");
	  osDelay(240);	//Completes the 1s period of measures
  }
  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN Header_startLeds */
/**
* @brief Function implementing the updateLeds thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startLeds */
void startLeds(void *argument)
{
  /* USER CODE BEGIN startLeds */
  /* Infinite loop */
  for(;;)
  {
	//printf("Led Task v1 Loop\n");
	  resetWSLED();
	  for (uint8_t i=0 ; i<9 ; i++) {
		  writeWSLED(WSdata[i]);
	  }


    osDelay(1000);
	  //userDelay(100, usTimerHandler);
  }
  /* USER CODE END startLeds */
}

/* USER CODE BEGIN Header_blinkingLED */
/**
* @brief Function implementing the blinkLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_blinkingLED */
void blinkingLED(void *argument)
{
  /* USER CODE BEGIN blinkingLED */
  /* Infinite loop */
  for(;;)
  {
	  //printf("UserSignal generated \n");
	  HAL_GPIO_TogglePin(userSignal2_GPIO_Port,userSignal2_Pin);
    //osDelay(1000);
    userDelay(100, usTimerHandler);
  }
  /* USER CODE END blinkingLED */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM11 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
