
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
// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */
#define RTC_BKP_DATE_TIME_UPDTATED ((uint32_t)0x32F2)
#define RTC_ASYNCH_PREDIV          ((uint32_t)0x7F)
#define RTC_SYNCH_PREDIV           ((uint32_t)0x00F9)
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
int expe = 2;
volatile _Bool blue_mode = false;

/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0};

void SysTick_Handler(void)  {

	//We do this whether expe 1 or expe 2 are being tested
	if((expe == 1) || (expe == 2)){
		if(i<5*expe){
			LED_GREEN(true);
		} else {
			LED_GREEN(false);
		}

		i++;
		if(i == 99){
			i = 0;
		}
	}

	//We do this only if expe 2 is being tested
	if(expe == 2){
		if(toggleState){
			LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
			toggleState = false;
		} else {
			LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);
			toggleState = true;
		}
	}

}

int main(void)
{

	_Bool isReady = false;
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
	LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, (expe == 8) ? 1 : expe);
    LL_RTC_EnableWriteProtection(RTC);
	// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
	LL_Init1msTick( SystemCoreClock );
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);


	SysTick_Config(SystemCoreClock / 100); //10ms

	while (1)
	{
		if(BLUE_BUTTON()){
			blue_mode = true;
		}

		if(blue_mode){
			//We do this whether expe 1 or expe 2 are being tested
			if((expe == 1) || (expe == 2)){
				LL_LPM_EnableSleep();
				__WFI();
			}

			//We do this only if expe 2 is being tested: LSE calibration
			if((expe == 2) && (isReady == false)){
				isReady = true;
				LL_RCC_MSI_EnablePLLMode();
			}
		}
	}
}

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

/**
 * @brief System Clock Configuration Selection
 * 		Here we chose the quartz configuration depending on the experience that is being tested
 *
 *
 * @param None
 * @retval None
 */
void SystemClock_ConfigSelection(void){

	switch (expe) {
	case 1:
		SystemClock_Config80MHz();
		break;

	case 2:
		SystemClock_Config24MHz();
		break;

	default: break;
	}
}


/**
  * @brief  Configure RTC clock.
  * @param  None
  * @retval None
  */
void Configure_RTC_Clock(void)
{
	/*##-1- Enables the PWR Clock and Enables access to the backup domain #######*/
	/* To change the source clock of the RTC feature (LSE, LSI), you have to:
     - Enable the power clock
     - Enable write access to configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain
     - Configure the needed RTC clock source */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_EnableBkUpAccess();

	/*##-2- Configure LSE/LSI as RTC clock source ###############################*/
	/* Enable LSE only if disabled.*/

	if (LL_RCC_LSE_IsReady() == 0)
	{
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();
		LL_RCC_LSE_Enable();

		while (LL_RCC_LSE_IsReady() != 1)
		{
		}
		LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
		Configure_RTC();
	}

}

/**
  * @brief  Configure RTC.
  * @note   Peripheral configuration is minimal configuration from reset values.
  *         Thus, some useless LL unitary functions calls below are provided as
  *         commented examples - setting is default configuration from reset.
  * @param  None
  * @retval None
  */
void Configure_RTC(void)
{
  /*##-1- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  LL_RCC_EnableRTC();

  /*##-2- Disable RTC registers write protection ##############################*/
  LL_RTC_DisableWriteProtection(RTC);

  /*##-3- Enter in initialization mode #######################################*/
  LL_RTC_EnableInitMode(RTC);

    while (LL_RTC_IsActiveFlag_INIT(RTC) != 1)
      {

      }

  /*##-4- Configure RTC ######################################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* Set Hour Format */
  LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_AMPM);
  /* Set Asynch Prediv (value according to source clock) */
  LL_RTC_SetAsynchPrescaler(RTC, RTC_ASYNCH_PREDIV);
  /* Set Synch Prediv (value according to source clock) */
  LL_RTC_SetSynchPrescaler(RTC, RTC_SYNCH_PREDIV);
  /* Set OutPut */
  /* Reset value is LL_RTC_ALARMOUT_DISABLE */
  //LL_RTC_SetAlarmOutEvent(RTC, LL_RTC_ALARMOUT_DISABLE);
  /* Set OutPutPolarity */
  /* Reset value is LL_RTC_OUTPUTPOLARITY_PIN_HIGH */
  //LL_RTC_SetOutputPolarity(RTC, LL_RTC_OUTPUTPOLARITY_PIN_HIGH);
  /* Set OutPutType */
  /* Reset value is LL_RTC_ALARM_OUTPUTTYPE_OPENDRAIN */
  //LL_RTC_SetAlarmOutputType(RTC, LL_RTC_ALARM_OUTPUTTYPE_OPENDRAIN);

  /*##-5- Exit of initialization mode #######################################*/
  LL_RTC_DisableInitMode(RTC);

  /*##-6- Enable RTC registers write protection #############################*/
  LL_RTC_EnableWriteProtection(RTC);
}
