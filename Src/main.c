
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stdbool.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include<stdio.h>
#include "clock.h"
#include "main.h"
#include "Rtc.h"

// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"

_Bool volatile toggleState = true;
void     SystemClock_Config(void);
void Show_RTC_Calendar(void);
void Configure_RTC_Calendar(void);
void Configure_RTC(void);
void Configure_RTC_Clock(void);
volatile int i = 0;

/*static prototypes*/
void SystemClock_ConfigSelection(void);

/*TP important global variables*/


volatile _Bool blue_mode = false;
_Bool isReady = false;

/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0};

void SysTick_Handler(void)  {



	//We do this whether expe 1 or expe 2 are being tested
	if(i<5*expe){
		LED_GREEN(true);
	} else {
		LED_GREEN(false);
	}

	i++;
	if(i == 99){
		i = 0;
	}

	if(toggleState){
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
		toggleState = false;
	} else {
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);
		toggleState = true;
	}


	if(BLUE_BUTTON()){
		blue_mode = true;
	}



	if(blue_mode){
		switch (expe){
		case 1:
			LL_LPM_EnableSleep();
			__WFI();
			break;
		case 2:
			if(isReady == false){
				isReady = true;
				LL_RCC_MSI_EnablePLLMode();
			}
			break;
		case 3:
			LL_LPM_EnableSleep();
			__WFI();
			break;
		case 4:
			if(isReady == false){
				isReady = true;
				LL_RCC_MSI_EnablePLLMode();
			}break;
		case 5:
			if(isReady == false){
				isReady = true;
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
				LL_LPM_EnableDeepSleep();
				__WFI();
			}
			break;
		case 6:
			if(isReady == false){
				isReady = true;
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
				LL_LPM_EnableDeepSleep();
				__WFI();
			}
			break;
		case 7:
			if(isReady == false){
				isReady = true;
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
				LL_LPM_EnableDeepSleep();
				__WFI();
			}
			break;
		case 8:
			LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
			LL_LPM_EnableDeepSleep();
			__WFI();
			break;
		default: break;
		}
	}

}

int main(void)
{


	/* Configure the system clock */
	SystemClock_ConfigSelection();

	// config GPIO
	GPIO_init();


	/*##-Configure the RTC peripheral #######################################*/
	Configure_RTC_Clock();
	LL_RTC_DisableWriteProtection(RTC);
	expe =  LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0);
	if(expe == 0){
		expe = 1;
	}
	expe ++;
	LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, (expe > 8) ? 1 : expe);
	LL_RTC_EnableWriteProtection(RTC);
	// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
	LL_Init1msTick( SystemCoreClock );
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);

	if(expe == 8){
		LL_RCC_MSI_EnablePLLMode();
		LL_LPM_EnableSleep();
		RTC_wakeup_init_from_standby_or_shutdown(20);
	}

	if((5 <= expe) || (expe >= 7)){
		LL_RCC_MSI_EnablePLLMode();
		LL_LPM_EnableSleep();
		RTC_wakeup_init_from_stop(20);
	}

	SysTick_Config(SystemCoreClock / 100); //10ms

	while (1)
	{

	}
}

/**
 * @brief System Clock Configuration Selection
 * 		Here we chose the quartz configuration depending on the experience that is being tested
 *
 *
 * @param None
 * @retval None
 */
void SystemClock_ConfigSelection(void){

	if(expe == 1){
		SystemClock_Config80MHz();
	} else {
		SystemClock_Config24MHz();
	}
}
