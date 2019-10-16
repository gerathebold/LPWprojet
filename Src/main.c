
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stdbool.h"
#include "stm32l4xx_ll_cortex.h"
// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"

_Bool volatile toggleState = true;
void     SystemClock_Config(void);
volatile int expe = 2;
volatile int i =0;
volatile _Bool ButtonState = false;
volatile int ConfigSelect = 1;
/*static prototypes*/
void SystemClock_ConfigSelection(void);


void SysTick_Handler(void)  {                               /* SysTick interrupt Handler. */

	if(i<5*expe){
		LED_GREEN(true);
	} else {
		LED_GREEN(false);
	}

	if(toggleState){
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
		toggleState = false;
	} else {
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);
		toggleState = true;
	}


	i++;
	if(i == 99){
		i = 0;
	}

}

int main(void)
{
	/* Configure the system clock */
	SystemClock_ConfigSelection();

	// config GPIO
	GPIO_init();

	// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
	LL_Init1msTick( SystemCoreClock );
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);


	SysTick_Config(SystemCoreClock / 100); //10ms

	while (1)
	{
		if(BLUE_BUTTON()){
			ButtonState = true;
		}

		if(ButtonState){
			LL_RCC_LSE_Enable();
			LL_LPM_EnableSleep();
			__WFI();
		}
	}
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
void SystemClock_Config(void)
{
	/* MSI configuration and activation */
	LL_RCC_MSI_EnableRangeSelection();
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
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

void SystemClock_ConfigSelection(void){

	switch (ConfigSelect) {
	case 1:
		SystemClock_Config();
		break;

	case 2:
		SystemClock_Config80MHz();
		break;

	default: break;
	}
}
