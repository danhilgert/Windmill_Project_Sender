/*
 * adc_app.h
 *
 * Created: 4/14/2017 7:47:26 PM
 *  Author: dan_h_000
 */ 


#ifndef ADC_APP_H_
#define ADC_APP_H_

 //! [result_buffer]
 #define ADC_SAMPLES 256
 uint16_t adc_result_buffer[ADC_SAMPLES];
 //! [result_buffer]

void adc_app_int(void);
uint8_t adc_app_get_flag (void);
void adc_app_reset_flag(void);
void adc_app_service(void);
uint16_t adc_app_get_value(void);


#endif /* ADC_APP_H_ */