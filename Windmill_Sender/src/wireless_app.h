/*
 * wireless_app.h
 *
 * Created: 4/14/2017 8:44:04 PM
 *  Author: dan_h_000
 */ 


#ifndef WIRELESS_APP_H_
#define WIRELESS_APP_H_

void wireless_app_set_length(uint8_t length);
void wireless_app_set_data(uint16_t adc_accumulated);
void APP_TaskHandler(void);

#endif /* WIRELESS_APP_H_ */