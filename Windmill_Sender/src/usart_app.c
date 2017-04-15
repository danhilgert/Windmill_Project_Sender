/*
 * usart_app.c
 *
 * Created: 4/14/2017 10:23:11 PM
 *  Author: dan_h_000
 */ 
#include <asf.h>
#include <stdio.h>

#define MODULE              SERCOM0
#define SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1
#define SERCOM_PINMUX_PAD0  PINMUX_PA04D_SERCOM0_PAD0
#define SERCOM_PINMUX_PAD1  PINMUX_PA05D_SERCOM0_PAD1
#define SERCOM_PINMUX_PAD2  PINMUX_UNUSED
#define SERCOM_PINMUX_PAD3  PINMUX_UNUSED
#define SERCOM_DMAC_ID_TX   SERCOM0_DMAC_ID_TX
#define SERCOM_DMAC_ID_RX   SERCOM0_DMAC_ID_RX

void usart_read_callback(struct usart_module *const usart_module);
void usart_write_callback(struct usart_module *const usart_module);

void configure_usart(void);
void configure_usart_callbacks(void);

static void configure_console(void);

struct usart_module usart_instance;
#define MAX_RX_BUFFER_LENGTH   5
volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];

 void usart_app_printf_init (void)
 {
	configure_console();
	configure_usart_callbacks();
 }
  static struct usart_module printf_usart_instance;
  static void configure_console(void)
  {
	  struct usart_config usart_conf;
	  usart_get_config_defaults(&usart_conf);
	  usart_conf.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	  usart_conf.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	  usart_conf.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	  usart_conf.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	  usart_conf.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	  usart_conf.baudrate    = 115200;

	  stdio_serial_init(&printf_usart_instance, EDBG_CDC_MODULE, &usart_conf);
	  usart_enable(&printf_usart_instance);
  }
 
  void usart_read_callback(struct usart_module *const usart_module)
  {
	  usart_write_buffer_job(&usart_instance,
	  (uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);
  }

  void usart_write_callback(struct usart_module *const usart_module)
  {
	  port_pin_toggle_output_level(LED_0_PIN);
  }
  
  void configure_usart_callbacks(void)
  {
	  //! [setup_register_callbacks]
	  usart_register_callback(&usart_instance,
	  usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	  usart_register_callback(&usart_instance,
	  usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
	  //! [setup_register_callbacks]

	  //! [setup_enable_callbacks]
	  usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_TRANSMITTED);
	  usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);
	  //! [setup_enable_callbacks]
  }
 