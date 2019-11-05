/*
 * Rtc.h
 *
 *  Created on: 5 nov. 2019
 *      Author: bouffard
 */

#ifndef SRC_RTC_H_
#define SRC_RTC_H_

void Configure_RTC_Clock(void);
void RTC_wakeup_init_from_standby_or_shutdown( int delay );
void RTC_wakeup_init_from_stop( int delay );
#endif /* SRC_RTC_H_ */
