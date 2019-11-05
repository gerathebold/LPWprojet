/*
 * clock.c
 *
 *  Created on: 5 nov. 2019
 *      Author: bouffard
 */

#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stdbool.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include "clock.h"
#include "main.h"

/*private prototypes*/
void Clock_setVoltage(void);

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 24000000
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
void SystemClock_Config24MHz(void)
{
	/* MSI configuration and activation */
	LL_RCC_MSI_EnableRangeSelection();
	if(expe > 2){
		LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
	} else {
		LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
	}
	Clock_setVoltage();
	LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9); //MSI=24Mhz
	LL_RCC_MSI_Enable();

	while	(LL_RCC_MSI_IsReady() != 1)
	{ };

	/* Set APB1 & APB2 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	/* Update the global variable called SystemCoreClock */
	SystemCoreClockUpdate();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config80MHz(void){
	/* MSI configuration and activation */
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
	LL_RCC_MSI_Enable();

	while	(LL_RCC_MSI_IsReady() != 1)
		{ };

	/* Main PLL configuration and activation */
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
	LL_RCC_PLL_Enable();
	LL_RCC_PLL_EnableDomain_SYS();
	while(LL_RCC_PLL_IsReady() != 1)
		{ };

	/* Sysclk activation on the main PLL */
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
		{ };

	/* Set APB1 & APB2 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	/* Update the global variable called SystemCoreClock */
	SystemCoreClockUpdate();
}

void Clock_setVoltage(void){
	if(expe > 2){
		LL_PWR_EnableLowPowerRunMode();
		LL_PWR_EnterLowPowerRunMode();
	}
}
