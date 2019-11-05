/*
 * Rtc.c
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
#include "stm32l4xx_ll_exti.h"
#include "Rtc.h"

#define RTC_BKP_DATE_TIME_UPDTATED ((uint32_t)0x32F2)
#define RTC_ASYNCH_PREDIV          ((uint32_t)0x7F)
#define RTC_SYNCH_PREDIV           ((uint32_t)0x00F9)
/*private prototypes*/
void Configure_RTC(void);


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

// partie commune a toutes les utilisations du wakeup timer
static void RTC_wakeup_init( int delay )
{
	LL_RTC_DisableWriteProtection( RTC );
	LL_RTC_WAKEUP_Disable( RTC );
	while	( !LL_RTC_IsActiveFlag_WUTW( RTC ) )
	{ }
	// connecter le timer a l'horloge 1Hz de la RTC
	LL_RTC_WAKEUP_SetClock( RTC, LL_RTC_WAKEUPCLOCK_CKSPRE );
	// fixer la duree de temporisation
	LL_RTC_WAKEUP_SetAutoReload( RTC, delay );	// 16 bits
	LL_RTC_ClearFlag_WUT(RTC);
	LL_RTC_EnableIT_WUT(RTC);
	LL_RTC_WAKEUP_Enable(RTC);
	LL_RTC_EnableWriteProtection(RTC);
}

// Dans le cas des modes STANDBY et SHUTDOWN, le MPU sera reveille par reset
// causé par 1 wakeup line (interne ou externe) (le NVIC n'est plus alimenté)
void RTC_wakeup_init_from_standby_or_shutdown( int delay )
{
	RTC_wakeup_init( delay );
	// enable the Internal Wake-up line
	LL_PWR_EnableInternWU();	// ceci ne concerne que Standby et Shutdown, pas STOPx
}

// Dans le cas des modes STOPx, le MPU sera reveille par interruption
// le module EXTI et une partie du NVIC sont encore alimentes
// le contenu de la RAM et des registres étant préservé, le MPU
// reprend l'execution après l'instruction WFI
void RTC_wakeup_init_from_stop( int delay )
{
	RTC_wakeup_init( delay );
	// valider l'interrupt par la ligne 20 du module EXTI, qui est réservée au wakeup timer
	LL_EXTI_EnableIT_0_31( LL_EXTI_LINE_20 );
	LL_EXTI_EnableRisingTrig_0_31( LL_EXTI_LINE_20 );
	// valider l'interrupt chez NVIC
	NVIC_SetPriority( RTC_WKUP_IRQn, 1 );
	NVIC_EnableIRQ( RTC_WKUP_IRQn );
}

// wakeup timer interrupt Handler (inutile mais doit etre defini)
void RTC_WKUP_IRQHandler()
{
	LL_EXTI_ClearFlag_0_31( LL_EXTI_LINE_20 );
}
