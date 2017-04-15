/*
 * timer_app.c
 *
 * Created: 4/14/2017 10:24:43 PM
 *  Author: dan_h_000
 */ 

#include <asf.h>
#include "timer_app.h"


static void configure_tcc(void);
static void configure_tcc_callbacks(void);
static void tcc_callback_ms(struct tcc_module *const module_inst);

struct tcc_module tcc_instance;

volatile uint32_t msecs = 0;

void timer_app_init(void)
{
configure_tcc();
configure_tcc_callbacks();
}

static void tcc_callback_ms(struct tcc_module *const module_inst)
{
	msecs++;
	port_pin_toggle_output_level(LED0_PIN);
}



static void configure_tcc(void)
{
	struct tcc_config config_tcc;

	tcc_get_config_defaults(&config_tcc, TCC0);

	config_tcc.counter.clock_source = GCLK_GENERATOR_0;
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	config_tcc.counter.period =   47619;

	tcc_init(&tcc_instance, TCC0, &config_tcc);
	tcc_enable(&tcc_instance);
}

static void configure_tcc_callbacks(void)
{
	tcc_register_callback(&tcc_instance, tcc_callback_ms,TCC_CALLBACK_OVERFLOW);
	tcc_enable_callback(&tcc_instance, TCC_CALLBACK_OVERFLOW);
}

void timer_app_service (void)
{
	uint8_t i;
	while(msecs)
	{
		for( i = 0; i < NUM_TIMERS; i++)
		{
			if(app_timers[i])
			{
				app_timers[i]--;
			}	
		}
			

		Disable_global_interrupt();
		msecs--;
		Enable_global_interrupt();
	}	
}

void timer_app_set(TIMERS_t timer, TIMER_COUNTS_t count)
{
	app_timers[timer] = count;
}

uint32_t timer_app_get_timer(TIMERS_t timer)
{
	return app_timers[timer];
}