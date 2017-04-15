/*
 * adc_app.c
 *
 * Created: 4/14/2017 7:47:11 PM
 *  Author: dan_h_000
 */ 
 #include "asf.h"
 #include "adc_app.h"

 void configure_adc(void);
 void configure_adc_callbacks(void);
 void adc_complete_callback(struct adc_module *const module);

 //! [module_inst]
 struct adc_module adc_instance;
 //! [module_inst]

 //! [job_complete_callback]
 volatile bool adc_read_done = false;

 void adc_app_int (void)
 {
	configure_adc();
	configure_adc_callbacks();
	adc_read_buffer_job(&adc_instance, adc_result_buffer, ADC_SAMPLES);
 }
 //! [setup]
 void configure_adc(void)
 {
	 struct adc_config config_adc;
	 adc_get_config_defaults(&config_adc);
	 #if (!SAML21) && (!SAML22) && (!SAMC21) && (!SAMR30)
	 config_adc.gain_factor     = ADC_GAIN_FACTOR_DIV2;
	 #endif
	 config_adc.accumulate_samples = ADC_ACCUMULATE_SAMPLES_256;
	 config_adc.divide_result = ADC_DIVIDE_RESULT_16;
	 config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV512;
	 config_adc.reference       = ADC_REFERENCE_INTVCC1;
	 config_adc.positive_input  = ADC_POSITIVE_INPUT_PIN6;
	 config_adc.resolution      = ADC_RESOLUTION_CUSTOM;
	 adc_init(&adc_instance, ADC, &config_adc);

	 adc_enable(&adc_instance);
 }

 void configure_adc_callbacks(void)
 {
	 adc_register_callback(&adc_instance,adc_complete_callback, ADC_CALLBACK_READ_BUFFER);
	 adc_enable_callback(&adc_instance, ADC_CALLBACK_READ_BUFFER);
 }

  void adc_complete_callback(struct adc_module *const module)
  {
	  adc_read_done = true;
  }
  //! [job_complete_callback]

  uint8_t adc_app_get_flag (void)
  {
	return adc_read_done;
  }

  void adc_app_reset_flag (void)
  {
	adc_read_done = 0;
	adc_read_buffer_job(&adc_instance, adc_result_buffer, ADC_SAMPLES);
  }