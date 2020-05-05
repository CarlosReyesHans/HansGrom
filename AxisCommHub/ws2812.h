#ifndef __ws2812
#define __ws2812

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/* Definition of TIM instance */
#define TIMx                             	TIM1
//TODO all the definitions used within the MspInit can be erased once the app is tested
/* Definition for TIMx clock resources */
#define TIMx_CLK_ENABLE                  	__HAL_RCC_TIM2_CLK_ENABLE
#define DMAx_CLK_ENABLE                  	__HAL_RCC_DMA1_CLK_ENABLE

/* Definition for TIMx Pins */
#define TIMx_CHANNEL1_GPIO_CLK_ENABLE    	__HAL_RCC_GPIOA_CLK_ENABLE
#define TIMx_GPIO_CHANNEL1_PORT          	GPIOA
#define GPIO_PIN_CHANNEL1                	GPIO_PIN_5
#define GPIO_AF_TIMx                     	GPIO_AF1_TIM2

/* Definition for TIMx's DMA */
#define TIMx_CC1_DMA_REQUEST             	DMA_REQUEST_4
#define TIMx_CC1_DMA_INST                	DMA_CHANNEL_6

/* Definition for DMAx's NVIC */
#define TIMx_DMA_IRQn                    	DMA2_Stream4_IRQn//DMA_CHANNEL_6_IRQn
#define TIMx_DMA_IRQHandler              	DMA1_Channel5_IRQHandler

//WS2812
#define WS2812_FREQ												(800000) 			// it is fixed: WS2812 require 800kHz
#define TIMER_CLOCK_FREQ									(16000000)   	// can be modified - multiples of 0.8MHz are suggested
#define TIMER_PERIOD											(TIMER_CLOCK_FREQ / WS2812_FREQ)
#define LED_NUMBER												(16)					// how many LEDs the MCU should control?
#define LED_DATA_SIZE											(LED_NUMBER * 24)
#define RESET_SLOTS_BEGIN									(50)
#define RESET_SLOTS_END										(50)
#define WS2812_LAST_SLOT									(1)
#define LED_BUFFER_SIZE										(RESET_SLOTS_BEGIN + LED_DATA_SIZE + WS2812_LAST_SLOT + RESET_SLOTS_END)
#define WS2812_0													(33)//(TIMER_PERIOD / 3)				// TODO This is considering the current prescaler and base clock of 80Mhz WS2812's zero high time is long about one third of the period
#define WS2812_1													(66)//(TIMER_PERIOD * 2 / 3)		// WS2812's one high time is long about two thirds of the period
#define WS2812_RESET											(0)
static uint8_t LEDbuffer[LED_BUFFER_SIZE];
void ws2812_init(void);
void ws2812_update(TIM_HandleTypeDef* pwmTempHandler);
void setLEDcolor(uint32_t LEDnumber, uint8_t RED, uint8_t GREEN, uint8_t BLUE);
void setWHOLEcolor(uint8_t RED, uint8_t GREEN, uint8_t BLUE);
void fillBufferBlack(void);
void fillBufferWhite(void);
void TIMx_DMA_IRQHandler(void);

#endif
