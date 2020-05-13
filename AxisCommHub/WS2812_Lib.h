/**
 * WS2812 Neopixel LED driver for STM32, Header
 * @Author: Nicolas Dammin, 2016
 * 		Changes:
 * 		Time constants are adjusted to use case
 * 		TIMER and DMA handlers, mcu library
 */

#pragma once

#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim1;//htim2;
DMA_HandleTypeDef hdma_tim1_ch1;

#define WS2812_NUM_LEDS_CH1		70	//58)

#define  WS2812_TIM_PRESCALE    0  // F_T3  = 72 MHz (13.88ns)
#define  WS2812_TIM_PERIODE   	100-1  // F_PWM = 800 kHz (1.25us)

#define  WS2812_LO_TIME        	33  // 33 * 13,9ns = 0.43us
#define  WS2812_HI_TIME        	66  // 66 * 13.9ns = 0.81us

#define	WS2812_MIN_REFRESH_TIME		(1.25*24*WS2812_NUM_LEDS_CH1+50)/1000	//This is 2.15 ms for 70 LEDS
#define	WS2812_REFRESH_TIME			20		//ms, 33ms -> 30 Hz; 20 -> 50Hz : Each Refresh time the WSB2812 will be updated
//--------------------------------------------------------------
// RGB LED Farbdefinition (3 x 8bit)
//--------------------------------------------------------------
typedef struct {
  uint8_t red;    // 0...255 (als PWM-Wert)
  uint8_t green;  // 0...255 (als PWM-Wert)
  uint8_t blue;   // 0...255 (als PWM-Wert)
}WS2812_RGB_t;


//--------------------------------------------------------------
// HSV LED Farbdefinition
//--------------------------------------------------------------
typedef struct {
  uint16_t h;     // 0...359 (in Grad, 0=R, 120=G, 240=B)
  uint8_t s;      // 0...100 (in Prozent)
  uint8_t v;      // 0...100 (in Prozent)
}WS2812_HSV_t;

WS2812_RGB_t WS2812_LED_BUF_CH1[WS2812_NUM_LEDS_CH1];

#define WS2812_BUFLEN	(WS2812_NUM_LEDS_CH1+2)*24

//Interrupt Callback
void DMA_Callback(void);	//TODO check whether this is called

//Library Interface
void WS2812_Refresh(void);
void WS2812_Clear(void);
void WS2812_RGB2HSV(WS2812_HSV_t hsv_col, WS2812_RGB_t *rgb_col);
void WS2812_One_RGB(uint32_t nr, WS2812_RGB_t rgb_col, uint8_t refresh);
void WS2812_All_RGB(WS2812_RGB_t rgb_col, uint8_t refresh);
void WS2812_One_HSV(uint32_t nr, WS2812_HSV_t hsv_col, uint8_t refresh);
void WS2812_All_HSV(WS2812_HSV_t hsv_col, uint8_t refresh);
void WS2812_Shift_Left(uint8_t refresh);
void WS2812_Shift_Right(uint8_t refresh);
void WS2812_Rotate_Left(uint8_t refresh);
void WS2812_Rotate_Right(uint8_t refresh);
void startDMA(void);


