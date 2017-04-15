/*
 * main.c
 *
 * Created: 4/14/2017 8:20:10 PM
 *  Author: dan_h_000
 */ 

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "adc_app.h"
 #include "usart_app.h"
 #include "config.h"
 #include "sys.h"
 #include "adc_app.h"
 #include "wireless_app.h"
 #include "timer_app.h"
 #include "asf.h"
 #if SAMD || SAMR21 || SAML21
 #include "system.h"
 #else
 #include "led.h"
 #include "sysclk.h"
 #endif


 int main(void)
 {
	irq_initialize_vectors();

	system_init();
	delay_init();
	timer_app_init();
	adc_app_int();
	usart_app_printf_init ();
	system_interrupt_enable_global();
	//configure_port_pins();
	SYS_Init();
	timer_app_set(THIRTY_SEC, THIRTY_SEC_COUNT);
	cpu_irq_enable();
	LED_On(LED0);
	printf("System Running!\r\n");
	while (1) 
	{
		SYS_TaskHandler();
		APP_TaskHandler();
		adc_app_service();
		timer_app_service();
		
		if(!timer_app_get_timer(THIRTY_SEC))
		{
			timer_app_set(THIRTY_SEC, THIRTY_SEC_COUNT);
			wireless_app_set_data(adc_app_get_value());
		}
	}

}
