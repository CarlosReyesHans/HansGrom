/**
 * WS2812 Neopixel LED driver for STM32
 * Based upon the WS2812 Library for STM32F4 from Uwe Becker, http://mikrocontroller.bplaced.net/wordpress/?page_id=3665
 * @Author: Nicolas Dammin, 2016
 * Modifications to add a multi channel functionality
 * @Author Carlos Reyes, 2020
 */
#include "stm32f4xx_hal.h"
#include "WS2812_Lib_MultiChannel.h"
#include "AxisCommHub_definitions.h"
#include "cmsis_os.h"



//Multichannel
uint16_t WS2812_TIM_BUF1[WS2812_BUFLEN1];
uint16_t WS2812_TIM_BUF2[WS2812_BUFLEN2];

uint8_t dma_ready = 1;	//TODO delete this variables
uint8_t buffer_updated = 0;



/*------------------------------------------------------Global variables----------------------------------------------------------------------*/
// TIMER Handlers used in the library
static TIM_HandleTypeDef *ledCH1,*ledCH2,*ledCH3,*ledCH4;
static DMA_HandleTypeDef *dmaCH1,*dmaCH2,*dmaCH3,*dmaCH4;

volatile uint8_t currentColors[MAX_OF_LEDRINGS];	//Global array for colors to be updated, this will be changed continuously by EventHandler/Notification //CHCKME this is shared memory
volatile uint8_t dmaLed1_rcvd, dmaLed2_rcvd;
volatile uint8_t refreshTime;	//TODO This is a flag that could be replaced by a Timer or signals created by the OS
volatile uint8_t ledRing1Data[NUM_OF_LEDS_PER_RING],ledRing2Data[NUM_OF_LEDS_PER_RING];

/*-------------------------------------------------Extern variables from other SMs-------------------------------------------------------------*/
extern osEventFlagsId_t evt_sysSignals;


/*----------------------------------------------------Functions------------------------------------------------------------------------*/
/**
 * @brief	This callback belongs to DMA after has finished to push the buffer.
 * 				Keep in mind that refresh function does not run if dma has not sent the data.
 *
 */

void DMA_Callback(void) {
	dma_ready = 1;
	buffer_updated = 0;
}

/**
 * @brief	Internal function, calculates the HI or LO values for the 800 kHz WS2812 signal and puts them into a buffer for the Timer-DMA
 * 				After the update the global buffer_updated flag is set.
 * @param	uint8_t ch:	Channel to be updated
 * @retval	1 if updated, -1 by error
 *
 */
int8_t calcBuf(uint8_t ch) {
  uint32_t n;
  uint32_t pos = 0;
  WS2812_RGB_t led,*WS2812_COLOR_BUF_CHX;
  uint16_t* bufferPtr;
  uint8_t tempNrLeds;

  if (ch > MAX_OF_LEDRINGS) return -1;

  switch (ch) {
  case 1:
	  bufferPtr = WS2812_TIM_BUF1;
	  tempNrLeds = WS2812_NUM_LEDS_CH1;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH1;
	  break;
  case 2:
	  bufferPtr = WS2812_TIM_BUF2;
	  tempNrLeds = WS2812_NUM_LEDS_CH2;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH2;
	  break;
  case 3:
	  __NOP(); //
	  break;
  case 4:
	  __NOP(); //
  	  break;
  default:
	  __NOP();
  }


  // set timings for all LEDs
  for(n=0;n<tempNrLeds;n++) {\
    led=WS2812_COLOR_BUF_CHX[n];

    // Col:Green , Bit:7..0
    bufferPtr[pos++]=((led.green&0x80) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x40) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x20) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x10) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x08) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x04) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x02) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.green&0x01) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;

    // Col:Red , Bit:7..0
    bufferPtr[pos++]=((led.red&0x80) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x40) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x20) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x10) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x08) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x04) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x02) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.red&0x01) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;

	// Col:Blue , Bit:7..0
    bufferPtr[pos++]=((led.blue&0x80) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x40) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x20) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x10) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x08) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x04) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x02) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
    bufferPtr[pos++]=((led.blue&0x01) != 0)?WS2812_HI_TIME:WS2812_LO_TIME;
  }

  // short pause after all LEDs have been updated
  for(n=0;n<48;n++) {
	  bufferPtr[pos++]=0;
  }
  //	updates the global flag for buffer updated
  //buffer_updated = 1;
  return 1;
}


/* *
 * @brief	Starts the transmission of data as PWM+DMA on the channel that is specified. Uses the library defined buffers
 * 				CHCKME	The channel of the timer is hardcoded and needs to be changed depending on Hardware
 * @param	channel over which the data is being sent
 * @retval	1 if HAL returned OK, -1 for any other event TIMEOUT, ERROR or BUSY
 *
 * */

int8_t ledDMA_send(uint8_t ch) {
	//uint8_t test[8] = {10};
	//dma_ready = 0; chckme this may not be needed any longer
	HAL_StatusTypeDef tempStatus;
		if (ch > MAX_OF_LEDRINGS) return -1;

		switch (ch) {
		case 1:
			tempStatus = HAL_TIM_PWM_Start_DMA(ledCH1, TIM_CHANNEL_1, (uint32_t *)WS2812_TIM_BUF1, WS2812_BUFLEN1);	//PENDING	Extend for more than 4 channels
			break;
		case 2:
			tempStatus = HAL_TIM_PWM_Start_DMA(ledCH2, TIM_CHANNEL_3, (uint32_t *)WS2812_TIM_BUF2, WS2812_BUFLEN2);
		  break;
		case 3:
		  __NOP(); //
		  break;
		case 4:
		  __NOP(); //TODO
		  break;
		default:
		  __NOP();
		}

		if (tempStatus == HAL_OK) return 1;
		else return -1;

//	HAL_TIM_PWM_Start_DMA(ledCH3, TIM_CHANNEL_3, (uint32_t *)WS2812_TIM_BUF1, WS2812_BUFLEN1);
//	HAL_TIM_PWM_Start_DMA(ledCH4, TIM_CHANNEL_4, (uint32_t *)WS2812_TIM_BUF2, WS2812_BUFLEN2);


}

void WS2812_Refresh(uint8_t ch) {

//	while(!dma_ready);
	calcBuf(ch);
	//ledDMA_send();		//CHCKME It is possible that this function is not needed
	__NOP();
}

/**
 * @brief Set all LEDs to 0 (off) on the prefered channel. Does not update the pwm buffer automatically.
 * @param ch
 */
int8_t WS2812_Clear(uint8_t ch) {
	uint16_t num;
	WS2812_RGB_t *WS2812_COLOR_BUF_CHX;
	uint8_t tempNrLeds;

	if (ch > MAX_OF_LEDRINGS) return -1;

	switch (ch) {
	case 1:
	  tempNrLeds = WS2812_NUM_LEDS_CH1;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH1;
	  break;
	case 2:
	  tempNrLeds = WS2812_NUM_LEDS_CH2;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH2;
	  break;
	case 3:
	  __NOP(); //
	  break;
	case 4:
	  __NOP(); //TODO
	  break;
	default:
	  __NOP();
	}


	for(num = 0; num < tempNrLeds; num++) {
		WS2812_COLOR_BUF_CHX[num] = (WS2812_RGB_t){0,0,0};
	}

	//WS2812_Refresh();
	return 1;
}

/**
 * Convert HSV-Value to RGB Value for WS2812 LEDs
 * (from www.ulrichradig.de)
 */
void WS2812_RGB2HSV(WS2812_HSV_t hsv_col, WS2812_RGB_t *rgb_col)
{
  uint8_t diff;

  // Grenzwerte
  if(hsv_col.h>359) hsv_col.h=359;
  if(hsv_col.s>100) hsv_col.s=100;
  if(hsv_col.v>100) hsv_col.v=100;

  if(hsv_col.h < 61) {
    rgb_col->red = 255;
    rgb_col->green = (425 * hsv_col.h) / 100;
    rgb_col->blue = 0;
  }else if(hsv_col.h < 121){
    rgb_col->red = 255 - ((425 * (hsv_col.h-60))/100);
    rgb_col->green = 255;
    rgb_col->blue = 0;
  }else if(hsv_col.h < 181){
    rgb_col->red = 0;
    rgb_col->green = 255;
    rgb_col->blue = (425 * (hsv_col.h-120))/100;
  }else if(hsv_col.h < 241){
    rgb_col->red = 0;
    rgb_col->green = 255 - ((425 * (hsv_col.h-180))/100);
    rgb_col->blue = 255;
  }else if(hsv_col.h < 301){
    rgb_col->red = (425 * (hsv_col.h-240))/100;
    rgb_col->green = 0;
    rgb_col->blue = 255;
  }else {
    rgb_col->red = 255;
    rgb_col->green = 0;
    rgb_col->blue = 255 - ((425 * (hsv_col.h-300))/100);
  }

  hsv_col.s = 100 - hsv_col.s;
  diff = ((255 - rgb_col->red) * hsv_col.s)/100;
  rgb_col->red = rgb_col->red + diff;
  diff = ((255 - rgb_col->green) * hsv_col.s)/100;
  rgb_col->green = rgb_col->green + diff;
  diff = ((255 - rgb_col->blue) * hsv_col.s)/100;
  rgb_col->blue = rgb_col->blue + diff;

  rgb_col->red = (rgb_col->red * hsv_col.v)/100;
  rgb_col->green = (rgb_col->green * hsv_col.v)/100;
  rgb_col->blue = (rgb_col->blue * hsv_col.v)/100;
}

/**
 * Set one LED (R, G, B values). If refresh == 1, update LEDs, otherwise just update buffer (if several function calls are to be done before refresh)
 */
//void WS2812_One_RGB(uint32_t nr, WS2812_RGB_t rgb_col, uint8_t refresh)
//{
//  if(nr<WS2812_NUM_LEDS_CH1) {
//	  WS2812_COLOR_BUF_CH1[nr]=rgb_col;
//
//    if(refresh==1) WS2812_Refresh();
//  }
//}

/**
 * Set all LEDs (R, G, B values). If refresh == 1, update LEDs, otherwise just update buffer (if several function calls are to be done before refresh)
 */
int8_t WS2812_All_RGB(uint8_t ch,WS2812_RGB_t rgb_col, uint8_t refresh) {
	WS2812_RGB_t *WS2812_COLOR_BUF_CHX;
	uint8_t tempNrLeds;

	if (ch > MAX_OF_LEDRINGS) return -1;

	switch (ch) {
	case 1:
	  tempNrLeds = WS2812_NUM_LEDS_CH1;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH1;
	  break;
	case 2:
	  tempNrLeds = WS2812_NUM_LEDS_CH2;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH2;
	  break;
	case 3:
	  __NOP(); //
	  break;
	case 4:
	  __NOP(); //TODO
	  break;
	default:
	  __NOP();
	}

  for(uint32_t n=0;n<tempNrLeds;n++) {
	  WS2812_COLOR_BUF_CHX[n]=rgb_col;
  }
  if(refresh==1) WS2812_Refresh(ch);
}

/**
 * Set one LED (H, S, V values). If refresh == 1, update LEDs, otherwise just update buffer (if several function calls are to be done before refresh)
 */
//void WS2812_One_HSV(uint32_t nr, WS2812_HSV_t hsv_col, uint8_t refresh)
//{
//  WS2812_RGB_t rgb_col;
//
//  if(nr<WS2812_NUM_LEDS_CH1) {
//    // convert to RGB
//    WS2812_RGB2HSV(hsv_col, &rgb_col);
//    WS2812_COLOR_BUF_CH1[nr]=rgb_col;
//
//    if(refresh==1) WS2812_Refresh();
//  }
//}

/**
 * Set all LEDs (H, S, V values). If refresh == 1, update LEDs, otherwise just update buffer (if several function calls are to be done before refresh)
 */
int8_t WS2812_All_HSV(uint8_t ch, WS2812_HSV_t hsv_col, uint8_t refresh)
{
  uint32_t n;
  WS2812_RGB_t rgb_col;


	WS2812_RGB_t *WS2812_COLOR_BUF_CHX;
	uint8_t tempNrLeds;

	if (ch > MAX_OF_LEDRINGS) return -1;

	switch (ch) {
	case 1:
	  tempNrLeds = WS2812_NUM_LEDS_CH1;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH1;
	  break;
	case 2:
	  tempNrLeds = WS2812_NUM_LEDS_CH2;
	  WS2812_COLOR_BUF_CHX = WS2812_COLOR_BUF_CH2;
	  break;
	case 3:
	  __NOP(); //
	  break;
	case 4:
	  __NOP(); //TODO
	  break;
	default:
	  __NOP();
	}

  // convert to RGB
  WS2812_RGB2HSV(hsv_col, &rgb_col);
  for(n=0;n<tempNrLeds;n++) {
	  WS2812_COLOR_BUF_CHX[n]=rgb_col;
  }
  if(refresh==1) WS2812_Refresh(ch);
  return 1;
}

/**
 * Shift all LED values one to the left. Last one will be turned off
 */
//void WS2812_Shift_Left(uint8_t refresh)
//{
//  uint32_t n;
//
//  if(WS2812_NUM_LEDS_CH1>1) {
//    for(n=1;n<WS2812_NUM_LEDS_CH1;n++) {
//    	WS2812_COLOR_BUF_CH1[n-1]=WS2812_COLOR_BUF_CH1[n];
//    }
//    WS2812_COLOR_BUF_CH1[n-1]=(WS2812_RGB_t){0,0,0};
//
//    if(refresh==1) WS2812_Refresh();
//  }
//}

/**
 * Shift all LED values one to the right. First one will be turned off
 */
//void WS2812_Shift_Right(uint8_t refresh)
//{
//  uint32_t n;
//
//  if(WS2812_NUM_LEDS_CH1>1) {
//    for(n=WS2812_NUM_LEDS_CH1-1;n>0;n--) {
//    	WS2812_COLOR_BUF_CH1[n]=WS2812_COLOR_BUF_CH1[n-1];
//    }
//    WS2812_COLOR_BUF_CH1[n]=(WS2812_RGB_t){0,0,0};
//
//    if(refresh==1) WS2812_Refresh();
//  }
//}

/**
 * Shift all LED values one to the left. Last LED value will be the previous first value
 */
//void WS2812_Rotate_Left(uint8_t refresh)
//{
//  uint32_t n;
//  WS2812_RGB_t d;
//
//  if(WS2812_NUM_LEDS_CH1>1) {
//    d=WS2812_COLOR_BUF_CH1[0];
//    for(n=1;n<WS2812_NUM_LEDS_CH1;n++) {
//    	WS2812_COLOR_BUF_CH1[n-1]=WS2812_COLOR_BUF_CH1[n];
//    }
//    WS2812_COLOR_BUF_CH1[n-1]=d;
//
//    if(refresh==1) WS2812_Refresh();
//  }
//}

/**
 * Shift all LED values one to the right. First LED value will be the previous last value
 */
//void WS2812_Rotate_Right(uint8_t refresh)
//{
//  uint32_t n;
//  WS2812_RGB_t d;
//
//  if(WS2812_NUM_LEDS_CH1>1) {
//    d=WS2812_COLOR_BUF_CH1[WS2812_NUM_LEDS_CH1-1];
//    for(n=WS2812_NUM_LEDS_CH1-1;n>0;n--) {
//    	WS2812_COLOR_BUF_CH1[n]=WS2812_COLOR_BUF_CH1[n-1];
//    }
//    WS2812_COLOR_BUF_CH1[n]=d;
//
//    if(refresh==1) WS2812_Refresh();
//  }
//}

/**
 * @brief	De-initialize a PWM at the given channel
 * */
void ledDMA_deinit (uint8_t ch) {
	__NOP();
}	//PENDING Define correctly  and Change to a pwm handler


/* *
 * @brief Modifies the currentColorsArray to the initial values and refreshes the buffer to be sent
 * */
void led_setInitColors(void) {
	WS2812_RGB_t tempRGB = {0,0,255};
	WS2812_All_RGB(1, tempRGB, TRUE);		//Each channel can be set individually to the initial color
	tempRGB = (WS2812_RGB_t){0,255,255};
	WS2812_All_RGB(2, tempRGB, TRUE);
}

/**
 * @brief	Configure a PWM at the given channel
 * */
int8_t ledDMA_configCh (uint8_t ch,TIM_HandleTypeDef *handlerPtr,DMA_HandleTypeDef *dmaHandlerptr) {
	if (handlerPtr == NULL) return -1;

	switch (ch) {
	case	1:
		ledCH1 = handlerPtr;	//NOTE: Both LEDS work with the same timer, only the pwm channel changes //PENDING it could be necessary to use 2 timers?

		//		dmaCH1 = dmaHandlerptr;
//		dmaCH1->XferCpltCallback = dmaCallback_led1;	//PENDING To delete if it is not working
		break;
	case 	2:
		ledCH2 = handlerPtr;
		break;
	default:
		__NOP();
	}

	return 1;
}

/* *
 * @brief	This is the dma callback function for LED 1
 * */

void dmaCallback_led1 (DMA_HandleTypeDef *dmaHandlerptr) {
	//do something
	dmaLed1_rcvd = TRUE;
	checkAllDmaRdy();	//This cannot have any race condition because it is call only from interruptions
}

/* *
 * @brief	This is the dma callback function for LED 2
 * */

void dmaCallback_led2(void * argument) {
	//do something
	dmaLed2_rcvd = TRUE;
	checkAllDmaRdy();

}

/* *
 * @brief	Checks for dma global flags and send event if they are all set.
 * */
void checkAllDmaRdy(void) {
	if (dmaLed1_rcvd && dmaLed2_rcvd)	//PENDING To add conditions for 4 leds if needed
		osEventFlagsSet(evt_sysSignals, LED_EVENT);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	DMA_HandleTypeDef temp;	//TODO Delete, only for debugging purposes
	dmaCallback_led1 (&temp);
}
