/*
 * freq_measure_app.c
 *
 * Created: 4/16/2017 11:17:26 AM
 *  Author: dan_h_000
 */ 
 #include "asf.h"
 #include "freq_measure_app.h"
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

 void configure_extint_channel(void);
 void configure_extint_callbacks(void);
 void extint_detection_callback(void);
 void extint_detection_callback(void);
static void configure_event_channel(struct events_resource *resource);
static void configure_event_user(struct events_resource *resource);
void setup(void);
void TC3_ISR_Handler(void);
void configure_tc_callbacks(void);
static void configure_tc(void);

 // Setup TC3 to capture pulse-width and period

 struct events_resource freq_measure;

 #define TC_MODULE TC5
 struct tc_module tc_instance;

 #define EVENT_GENERATOR    EVSYS_ID_GEN_EIC_EXTINT_4
 #define EVENT_USER         EVSYS_ID_USER_TC5_EVU
 #define PA04_EIC_LINE	4

 void freq_measure_app_init(void)
 {
	configure_extint_channel();
	configure_tc();
	configure_event_channel(&freq_measure);
	configure_event_user(&freq_measure);
	configure_tc_callbacks();
	
	//configure_extint_callbacks();
 }

 void configure_extint_channel(void)
 {
	 struct extint_chan_conf config_extint_chan;
	 //REG_PM_APBCMASK |= PM_APBCMASK_EVSYS;     // Switch on the event system peripheral
	 extint_chan_get_config_defaults(&config_extint_chan);
	 config_extint_chan.gpio_pin           =  PIN_PA04A_EIC_EXTINT4;
	 config_extint_chan.gpio_pin_mux       =  MUX_PA04A_EIC_EXTINT4;
	 config_extint_chan.detection_criteria = EXTINT_DETECT_HIGH;
	 extint_chan_set_config(PA04_EIC_LINE, &config_extint_chan);
	 REG_EIC_EVCTRL |= EIC_EVCTRL_EXTINTEO4;      // Enable event output on external interrupt 4
	  /* Configure external interrupt module to be event generator */	  
	/* Configure external interrupt module to be event generator */
	struct extint_events extint_event_conf;
	extint_event_conf.generate_event_on_detect[4] = true;
	extint_enable_events(&extint_event_conf);
	
 }

 void configure_extint_callbacks(void)
 {
	 extint_register_callback(extint_detection_callback, PA04_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	 extint_chan_enable_callback(PA04_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
 }

 void extint_detection_callback(void)
 {
	 port_pin_toggle_output_level(LED_0_PIN);
 }
 
 static void configure_event_channel(struct events_resource *resource)
 {
	 struct events_config config;
	 events_get_config_defaults(&config);

	 config.generator      = EVENT_GENERATOR;
	 config.edge_detect    = EVENTS_EDGE_DETECT_NONE;
	 config.path           = EVENTS_PATH_ASYNCHRONOUS;
	 config.clock_source   = GCLK_GENERATOR_0;
	 events_allocate(resource, &config);
 }

 static void configure_event_user(struct events_resource *resource)
 {
	 events_attach_user(resource, EVENT_USER);
 }

 static void configure_tc(void)
 {
	 struct tc_config config_tc;
	 struct tc_events tc_event;
	 tc_get_config_defaults(&config_tc);
	 config_tc.counter_size    = TC_COUNTER_SIZE_16BIT;
	 config_tc.clock_source    = GCLK_GENERATOR_0;
	 config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV64;
	 config_tc.enable_capture_on_channel[TC_COMPARE_CAPTURE_CHANNEL_0] = true;
	 config_tc.enable_capture_on_channel[TC_COMPARE_CAPTURE_CHANNEL_1] = true;
	 tc_event.invert_event_input = false;
	 tc_event.on_event_perform_action = true;
	 tc_event.event_action = TC_EVENT_ACTION_PPW;
	 NVIC_SetPriority(TC5_IRQn, 0);      // Set the Nested Vector Interrupt Controller (NVIC) priority for TC3 to 0 (highest)
	 NVIC_EnableIRQ(TC5_IRQn);           // Connect the TC3 timer to the Nested Vector Interrupt Controller (NVIC)
	 tc_init(&tc_instance, TC_MODULE, &config_tc);
	 tc_enable_events(&tc_instance, &tc_event);
	 tc_enable(&tc_instance);
	

 }
 
 void configure_tc_callbacks(void)
 {
	
	//tc_register_callback(&tc_instance, TC3_ISR_Handler, TC_CALLBACK_CC_CHANNEL1); 
	tc_register_callback(&tc_instance, TC3_ISR_Handler,TC_CALLBACK_CC_CHANNEL0);

	//tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
	tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
 }
 //! [setup]

 static uint32_t period_avg = 0;
 static uint32_t pulse_width_avg = 0;

	void TC3_ISR_Handler(void)                                // Interrupt Service Routine (ISR) for timer TC3
	{
		volatile static uint16_t x = 0;
		volatile static uint16_t y = 0;
		volatile static uint32_t isrPeriod;
		volatile static uint32_t isrPulsewidth;
		
		// Check for match counter 0 (MC0) interrupt
		if (TC5->COUNT16.INTFLAG.bit.MC0)
		{
			REG_TC5_READREQ = TC_READREQ_RREQ |				// Enable a read request
			TC_READREQ_ADDR(0x18);							// Offset address of the CC0 register
			while (TC5->COUNT16.STATUS.bit.SYNCBUSY);		// Wait for (read) synchronization
			isrPeriod += REG_TC5_COUNT16_CC0;				// Copy the period
			x++;
			if (x > 1023)
			{
				period_avg = isrPeriod/1024;
				//printf("period avg = %d\r\n",period_avg);
				isrPeriod = 0;
				x = 0;
			}
			
		}
		
		// Check for match counter 1 (MC1) interrupt
		if (TC5->COUNT16.INTFLAG.bit.MC1)
		{
			REG_TC5_READREQ = TC_READREQ_RREQ |			// Enable a read request
			TC_READREQ_ADDR(0x1A);							// Offset address of the CC1 register
			while (TC5->COUNT16.STATUS.bit.SYNCBUSY);		// Wait for (read) synchronization
			isrPulsewidth += REG_TC5_COUNT16_CC1;			// Copy the pulse-width
			y++;
			if (y > 1023)
			{
				pulse_width_avg = isrPulsewidth/1024;
				//printf("pulse width avg = %d\r\n",pulse_width_avg);
				isrPulsewidth = 0;
				y = 0;
			}
			
		}
}


uint16_t freq_measure_app_get_pulse_width (void)
{
	return period_avg; 
}


 
 


