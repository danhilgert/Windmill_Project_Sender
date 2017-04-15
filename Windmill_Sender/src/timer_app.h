/*
 * timer_app.h
 *
 * Created: 4/14/2017 10:24:13 PM
 *  Author: dan_h_000
 */ 


#ifndef TIMER_APP_H_
#define TIMER_APP_H_

typedef enum{
	HUNDRED_MS,
	ONE_SEC,
	TEN_SEC,
	THIRTY_SEC,
	NUM_TIMERS

}TIMERS_t;

typedef enum{
	HUNDRED_MS_COUNT =		100,
	ONE_SEC_COUNT =			1000,
	TEN_SEC_COUNT =			10000,
	THIRTY_SEC_COUNT =		30000
}TIMER_COUNTS_t;

uint32_t app_timers[NUM_TIMERS];

void timer_app_init(void);
void timer_app_set(TIMERS_t timer, TIMER_COUNTS_t count);
void timer_app_service(void);
uint32_t timer_app_get_timer(TIMERS_t timer);


#endif /* TIMER_APP_H_ */