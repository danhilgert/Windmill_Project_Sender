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
 #include "asf.h"
 #if SAMD || SAMR21 || SAML21
 #include "system.h"
 #else
 #include "led.h"
 #include "sysclk.h"
 #endif


 int main(void)
 {
	 //irq_initialize_vectors();

	 system_init();
	 delay_init();

	 adc_app_int();
	 system_interrupt_enable_global();
	 usart_app_printf_init ();
	 printf("System Running!");
	 //configure_port_pins();
	 SYS_Init();
	 //sio2host_init();
	 cpu_irq_enable();
	 LED_On(LED0);
	 while (1) {
		 SYS_TaskHandler();
		 APP_TaskHandler();
		 //! [start_adc_job]
		 
		 while (adc_app_get_flag() == true)
		 {
			uint32_t adc_accumulated = 0;
			 adc_app_reset_flag();
			 uint8_t i;
			 for(i = 0; i < ADC_SAMPLES; i++)
			 {
				 adc_accumulated += adc_result_buffer[i];

			 }
			 adc_accumulated = adc_accumulated >> 4;
			 wireless_app_set_data(adc_accumulated);
		 }
	 }

 }
