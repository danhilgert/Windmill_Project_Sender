/**
 * \file Peer2Peer.c
 *
 * \brief Peer2Peer application implementation
 *
 * Copyright (C) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *
 */

/**
 * \mainpage
 * \section preface Preface
 * This is the reference manual for the LWMesh Peer2Peer Application
 * The LightWeight Mesh Peer2Peer  implements a wireless UART application.Two nodes are used in this application
 * These two nodes must be configured with addresses 0x0001 and 0x0000 respectively.
 * To test this application,open a terminal for both the nodes.On entering text in the terminal the data is transmitted from one 
 * node to another node(0x0001 to 0x0000 and vice-versa)
 */
/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "sys.h"
#include "nwkDataReq.h"
#if SAMD || SAMR21 || SAML21
#include "system.h"
#else
#include "led.h"
#include "sysclk.h"
#endif
#include "phy.h"
#include "nwk.h"
#include "sysTimer.h"
#include "sio2host.h"
#include "asf.h"

/*- Definitions ------------------------------------------------------------*/
#ifdef NWK_ENABLE_SECURITY
  #define APP_BUFFER_SIZE     (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
  #define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

static uint16_t rx_data[APP_RX_BUF_SIZE];

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t {
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
} AppState_t;

/*- Prototypes -------------------------------------------------------------*/
static void appSendData(void);

/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;
static uint8_t sio_rx_length;
/*- Implementations --------------------------------------------------------*/
/*************************************************************************//**
*****************************************************************************/
void configure_adc(void);
void configure_adc_callbacks(void);
void adc_complete_callback(struct adc_module *const module);

//! [result_buffer]
#define ADC_SAMPLES 16
uint16_t adc_result_buffer[ADC_SAMPLES];
//! [result_buffer]

//! [module_inst]
struct adc_module adc_instance;
//! [module_inst]

//! [job_complete_callback]
volatile bool adc_read_done = false;

void adc_complete_callback(struct adc_module *const module)
{
	adc_read_done = true;
}
//! [job_complete_callback]

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
config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV128;
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

/*************************************************************************//**
*****************************************************************************/
/** aplication data transmit request callback **/
static void appDataConf(NWK_DataReq_t *req)
{
	adc_read_buffer_job(&adc_instance, adc_result_buffer, ADC_SAMPLES);
	appDataReqBusy = false;
	(void)req;
}

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void)
{
	if (appDataReqBusy || 0 == appUartBufferPtr) {
		return;
	}

	memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);

	appDataReq.dstAddr = 1 - APP_ADDR;
	appDataReq.dstEndpoint = APP_ENDPOINT;
	appDataReq.srcEndpoint = APP_ENDPOINT;
	appDataReq.options = NWK_OPT_ENABLE_SECURITY;
	appDataReq.data = appDataReqBuffer;
	appDataReq.size = appUartBufferPtr;
	appDataReq.confirm = appDataConf;
	NWK_DataReq(&appDataReq);

	appUartBufferPtr = 0;
	appDataReqBusy = true;
	LED_Toggle(LED0);
}

/*************************************************************************//**
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer)
{
	appSendData();
	(void)timer;
}

/*************************************************************************//**
*****************************************************************************/
/**this is a calback from the stack to inform app that data has been recieved.**/
static bool appDataInd(NWK_DataInd_t *ind)
{
	for (uint8_t i = 0; i < ind->size; i++) {
		sio2host_putchar(ind->data[i]);
	}
	LED_Toggle(LED0);
	return true;
}

/*************************************************************************//**
*****************************************************************************/
static void appInit(void)
{
	NWK_SetAddr(APP_ADDR);
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
	PHY_SetBand(APP_BAND);
	PHY_SetModulation(APP_MODULATION);
#endif
	PHY_SetRxState(true);

	NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

	appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
	appTimer.mode = SYS_TIMER_INTERVAL_MODE;
	appTimer.handler = appTimerHandler;
}

void configure_port_pins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(BUTTON_0_PIN, &config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED_0_PIN, &config_port_pin);
}


/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void)
{
	switch (appState) {
	case APP_STATE_INITIAL:
	{
		appInit();
		appState = APP_STATE_IDLE;
	}
	break;

	case APP_STATE_IDLE:
		break;

	default:
		break;
	}
		

	
	//sio_rx_length = sio2host_rx(rx_data, APP_RX_BUF_SIZE);
	if (sio_rx_length) {
		for (uint16_t i = 0; i < sio_rx_length; i++) {
			sio2host_putchar(appUartBuffer[i]);
			if (appUartBufferPtr == sizeof(appUartBuffer)) {
				appSendData();
			}

			if (appUartBufferPtr < sizeof(appUartBuffer)) {
				appUartBuffer[appUartBufferPtr++] = rx_data[i];
			}
		}
		sio_rx_length = 0;
		SYS_TimerStop(&appTimer);
		SYS_TimerStart(&appTimer);
	}
	

	

	
}
/*************************************************************************//**
*****************************************************************************/

int main(void)
{
	//irq_initialize_vectors();

	system_init();
	delay_init();

	configure_adc();
	configure_adc_callbacks();
	system_interrupt_enable_global();

	//configure_port_pins();
	SYS_Init();
	sio2host_init();
	cpu_irq_enable();
	LED_On(LED0);
	adc_read_buffer_job(&adc_instance, adc_result_buffer, ADC_SAMPLES);
	while (1) {
		SYS_TaskHandler();
		APP_TaskHandler();
		//! [start_adc_job]
		
		while (adc_read_done == true)
		{
			adc_read_done = false;
			uint8_t i;
			for(i = 0; i < ADC_SAMPLES; i++)
			{
				rx_data[i] = adc_result_buffer[i];

			}
			
			rx_data[16] = 0x0A;
			rx_data[17] = 0x0D;

			sio_rx_length = sizeof(rx_data);
		}
	}

}
