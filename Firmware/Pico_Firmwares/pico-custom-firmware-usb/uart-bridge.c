#include <boards/pico_w.h>
#include <hardware/irq.h>
#include <hardware/structs/sio.h>
#include <hardware/uart.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <string.h>
#include <tusb.h>
#include "hardware/pio.h"
#include "rx.pio.h"
#include "hardware/pwm.h"
//udp
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define  RECEIVER_IP  "192.168.0.105"
#define  RECEIVER_PORT 6001
struct udp_pcb  * upcb;
struct udp_pcb  * spcb;

#if !defined(MIN)
#define MIN(a, b) ((a > b) ? b : a)
#endif /* MIN */


#define BUFFER_SIZE 2560

#define DEF_BIT_RATE 1500000
#define DEF_STOP_BITS 1
#define DEF_PARITY 0
#define DEF_DATA_BITS 8


typedef struct {
	uart_inst_t *const inst;
	uint irq;
	void *irq_fn;
	uint8_t tx_pin;
	uint8_t rx_pin;
} uart_id_t;

typedef struct {
	cdc_line_coding_t usb_lc;
	cdc_line_coding_t uart_lc;
	mutex_t lc_mtx;
	uint8_t uart_buffer[BUFFER_SIZE];
	uint32_t uart_pos;
	mutex_t uart_mtx;
	uint8_t usb_buffer[BUFFER_SIZE];
	uint32_t usb_pos;
	mutex_t usb_mtx;
} uart_data_t;

void uart0_irq_fn(void);
void uart1_irq_fn(void);

const uart_id_t UART_ID[CFG_TUD_CDC] = {
	{
		.inst = uart0,
		.irq = UART0_IRQ,
		.irq_fn = &uart0_irq_fn,
		.tx_pin = 0,
		.rx_pin = 1,
	}, {
		.inst = uart1,
		.irq = UART1_IRQ,
		.irq_fn = &uart1_irq_fn,
		.tx_pin = 4,
		.rx_pin = 5,
	}
};

uart_data_t UART_DATA[CFG_TUD_CDC];

static inline uint databits_usb2uart(uint8_t data_bits)
{
	switch (data_bits) {
		case 5:
			return 5;
		case 6:
			return 6;
		case 7:
			return 7;
		default:
			return 8;
	}
}

static inline uart_parity_t parity_usb2uart(uint8_t usb_parity)
{
	switch (usb_parity) {
		case 1:
			return UART_PARITY_ODD;
		case 2:
			return UART_PARITY_EVEN;
		default:
			return UART_PARITY_NONE;
	}
}

static inline uint stopbits_usb2uart(uint8_t stop_bits)
{
	switch (stop_bits) {
		case 2:
			return 2;
		default:
			return 1;
	}
}

void update_uart_cfg(uint8_t itf)
{
	const uart_id_t *ui = &UART_ID[itf];
	uart_data_t *ud = &UART_DATA[itf];

	mutex_enter_blocking(&ud->lc_mtx);

	if (ud->usb_lc.bit_rate != ud->uart_lc.bit_rate) {
		uart_set_baudrate(ui->inst, ud->usb_lc.bit_rate);
		ud->uart_lc.bit_rate = ud->usb_lc.bit_rate;
	}

	if ((ud->usb_lc.stop_bits != ud->uart_lc.stop_bits) ||
	    (ud->usb_lc.parity != ud->uart_lc.parity) ||
	    (ud->usb_lc.data_bits != ud->uart_lc.data_bits)) {
		uart_set_format(ui->inst,
				databits_usb2uart(ud->usb_lc.data_bits),
				stopbits_usb2uart(ud->usb_lc.stop_bits),
				parity_usb2uart(ud->usb_lc.parity));
		ud->uart_lc.data_bits = ud->usb_lc.data_bits;
		ud->uart_lc.parity = ud->usb_lc.parity;
		ud->uart_lc.stop_bits = ud->usb_lc.stop_bits;
	}

	mutex_exit(&ud->lc_mtx);
}

void usb_read_bytes(uint8_t itf)
{
	uart_data_t *ud = &UART_DATA[itf];
	uint32_t len = tud_cdc_n_available(itf);

	if (len &&
	    mutex_try_enter(&ud->usb_mtx, NULL)) {
		len = MIN(len, BUFFER_SIZE - ud->usb_pos);
		if (len) {
			uint32_t count;

			count = tud_cdc_n_read(itf, &ud->usb_buffer[ud->usb_pos], len);
			ud->usb_pos += count;
		}

		mutex_exit(&ud->usb_mtx);
	}
}

void usb_write_bytes(uint8_t itf)
{
	uart_data_t *ud = &UART_DATA[itf];

	if (ud->uart_pos &&
	    mutex_try_enter(&ud->uart_mtx, NULL)) {
		uint32_t count;

		count = tud_cdc_n_write(itf, ud->uart_buffer, ud->uart_pos);
		if (count < ud->uart_pos)
			memmove(ud->uart_buffer, &ud->uart_buffer[count],
			       ud->uart_pos - count);
		ud->uart_pos -= count;

		mutex_exit(&ud->uart_mtx);

		if (count)
			tud_cdc_n_write_flush(itf);
	}
}

void usb_cdc_process(uint8_t itf)
{
	uart_data_t *ud = &UART_DATA[itf];

	mutex_enter_blocking(&ud->lc_mtx);
	tud_cdc_n_get_line_coding(itf, &ud->usb_lc);
	mutex_exit(&ud->lc_mtx);

	usb_read_bytes(itf);
	usb_write_bytes(itf);
}

void core1_entry(void)
{

	tusb_init();

	while (1) {
		int itf;
		int con = 0;

		tud_task();

		for (itf = 0; itf < CFG_TUD_CDC; itf++) {
			if (tud_cdc_n_connected(itf)) {
				con = 1;
				usb_cdc_process(itf);
			}

		}
	}
}

static inline void uart_read_bytes(uint8_t itf)
{
	uart_data_t *ud = &UART_DATA[itf];
	const uart_id_t *ui = &UART_ID[itf];

	if (uart_is_readable(ui->inst)) {
		mutex_enter_blocking(&ud->uart_mtx);

		while (uart_is_readable(ui->inst) &&
		       (ud->uart_pos < BUFFER_SIZE)) {
			ud->uart_buffer[ud->uart_pos] = uart_getc(ui->inst);
			ud->uart_pos++;
		}

		mutex_exit(&ud->uart_mtx);
	}
}

void uart0_irq_fn(void)
{
	uart_read_bytes(0);
}

void uart1_irq_fn(void)
{
	uart_read_bytes(1);
}

void uart_write_bytes(uint8_t itf)
{
	uart_data_t *ud = &UART_DATA[itf];

	if (ud->usb_pos &&
	    mutex_try_enter(&ud->usb_mtx, NULL)) {
		const uart_id_t *ui = &UART_ID[itf];
		uint32_t count = 0;

		while (uart_is_writable(ui->inst) &&
		       count < ud->usb_pos) {
			uart_putc_raw(ui->inst, ud->usb_buffer[count]);
			count++;
		}

		if (count < ud->usb_pos)
			memmove(ud->usb_buffer, &ud->usb_buffer[count],
			       ud->usb_pos - count);
		ud->usb_pos -= count;

		mutex_exit(&ud->usb_mtx);
	}
}

void init_uart_data(uint8_t itf)
{
	const uart_id_t *ui = &UART_ID[itf];
	uart_data_t *ud = &UART_DATA[itf];

	/* Pinmux */
	gpio_set_function(ui->tx_pin, GPIO_FUNC_UART);
	gpio_set_function(ui->rx_pin, GPIO_FUNC_UART);

	/* USB CDC LC */
	ud->usb_lc.bit_rate = DEF_BIT_RATE;
	ud->usb_lc.data_bits = DEF_DATA_BITS;
	ud->usb_lc.parity = DEF_PARITY;
	ud->usb_lc.stop_bits = DEF_STOP_BITS;

	/* UART LC */
	ud->uart_lc.bit_rate = DEF_BIT_RATE;
	ud->uart_lc.data_bits = DEF_DATA_BITS;
	ud->uart_lc.parity = DEF_PARITY;
	ud->uart_lc.stop_bits = DEF_STOP_BITS;

	/* Buffer */
	ud->uart_pos = 0;
	ud->usb_pos = 0;

	/* Mutex */
	mutex_init(&ud->lc_mtx);
	mutex_init(&ud->uart_mtx);
	mutex_init(&ud->usb_mtx);

	/* UART start */
	uart_init(ui->inst, ud->usb_lc.bit_rate);
	uart_set_hw_flow(ui->inst, false, false);
	uart_set_format(ui->inst, databits_usb2uart(ud->usb_lc.data_bits),
			stopbits_usb2uart(ud->usb_lc.stop_bits),
			parity_usb2uart(ud->usb_lc.parity));
	uart_set_fifo_enabled(ui->inst, false);

	/* UART RX Interrupt */
	irq_set_exclusive_handler(ui->irq, ui->irq_fn);
	irq_set_enabled(ui->irq, true);
	uart_set_irq_enables(ui->inst, true, false);
}

void uyar(void){
	// Uyarma sargısı H bridge çıkışları
	const uint enA = 6;
	const uint in1 = 20;
	const uint in2 = 21;
	const int sw = 26; //pin 22 ile pin26 arasına jumper atıldı
	const uint p11=11;
	const uint  frekans= 200; // her bir değer 1us
	
	gpio_init(22);
	gpio_set_dir(22, GPIO_IN);

	gpio_set_function(enA, GPIO_FUNC_PWM);
	gpio_set_function(in1, GPIO_FUNC_PWM);
	gpio_set_function(in2, GPIO_FUNC_PWM);
	gpio_set_function(sw, GPIO_FUNC_PWM);
	gpio_set_function(p11, GPIO_FUNC_PWM);


	uint inputs = pwm_gpio_to_slice_num(in1);
	uint enable = pwm_gpio_to_slice_num(enA);
	int pin26 = pwm_gpio_to_slice_num(sw);
	int pin11 = pwm_gpio_to_slice_num(p11);

	// 250 mhz/250 =1mhz clock, 
	pwm_set_clkdiv(inputs,250);
    pwm_set_wrap(inputs, frekans);

	pwm_set_clkdiv(enable,250);
    pwm_set_wrap(enable, frekans);

	pwm_set_clkdiv(pin26,250);
    pwm_set_wrap(pin26, frekans);
	
	pwm_set_clkdiv(pin11,250);
    pwm_set_wrap(pin11, frekans);

	//phase correct
	pwm_set_phase_correct(inputs, true);
	pwm_set_phase_correct(enable, false);
	pwm_set_phase_correct(pin26, false);
	pwm_set_phase_correct(pin11, false);

	pwm_set_output_polarity(inputs,false, true);
	pwm_set_output_polarity(enable,false, false);
	pwm_set_output_polarity(pin26,false, false);
	pwm_set_output_polarity(pin11,false, false);
    //PWM counter ayarı


	pwm_set_chan_level(inputs, PWM_CHAN_A, frekans/(frekans/40.0));
    pwm_set_chan_level(inputs, PWM_CHAN_B, frekans-frekans/(frekans/40.0));

	pwm_set_counter(enable, frekans/(frekans/40.0));
	pwm_set_chan_level(enable, PWM_CHAN_A, frekans/(frekans/80.0));
	pwm_set_chan_level(pin26, PWM_CHAN_A, frekans/(frekans/120.0)); //demodülator uyarma sinyalinden 40us ileri

	pwm_set_counter(pin11, frekans/(frekans/82.0));
	pwm_set_chan_level(pin11, PWM_CHAN_B, frekans/(frekans/80.0));

	// Set the PWM running
    pwm_set_enabled(inputs, true);
	pwm_set_enabled(enable, true);
	pwm_set_enabled(pin26, true);
	pwm_set_enabled(pin11, true);

}

void SendUDP(char * IP , int port, void * data, int data_size)
{
      ip_addr_t   destAddr;
      ip4addr_aton(IP,&destAddr);
      struct pbuf * p = pbuf_alloc(PBUF_TRANSPORT,data_size,PBUF_RAM);
      memcpy(p->payload,data,data_size);
      cyw43_arch_lwip_begin();
      udp_sendto(upcb,p,&destAddr,port);
      cyw43_arch_lwip_end();
      pbuf_free(p);
}

int main(void)
{
	int itf;
	set_sys_clock_khz(250000, false);
	unsigned int n=0;
	uint8_t recv[1024];
	usbd_serial_init();

	for (itf = 0; itf < CFG_TUD_CDC; itf++)
		init_uart_data(itf);

	gpio_init(13);
	gpio_set_dir(13, GPIO_OUT);

	gpio_put(13,1);

	// SM kurulumu- pin10'a ek rx kanalı
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &uart_rx_program);
    uart_rx_program_init(pio, sm, offset, 10, 1500000);

	//core1 usb işini yapacak
	multicore_launch_core1(core1_entry);

	//udp kurulumu
   	cyw43_arch_init();
	cyw43_arch_enable_sta_mode(); //sta=client mode oluyor
	cyw43_arch_wifi_connect_timeout_ms("ÜÇERLER(2.4)","333ER333", CYW43_AUTH_WPA2_AES_PSK, 30000);
	upcb = udp_new();
	spcb = udp_new();
	err_t err = udp_bind(spcb,IP_ADDR_ANY,6001);

	uyar();
	
	while (1) {        		
		for (itf = 0; itf < CFG_TUD_CDC; itf++) {
			//usb üzerinden uart ayarları değişirse
			update_uart_cfg(itf);
			
			//çift yönlü uartta usb bufferından aldığını yazması için
			uart_write_bytes(itf);
		}

		//SM ile pin 10'a ek RX pinin eklenip uart1 bufferına yazılması

			while (!pio_sm_is_rx_fifo_empty(pio0, sm) && n < 1024) {
			recv[n]= uart_rx_program_getc(pio, sm);
			n++;
			}
			if(n>=1023){
			SendUDP(RECEIVER_IP,RECEIVER_PORT, recv, 1024);
			n=0;
			cyw43_arch_poll();

			}

	
	}

	return 0;
}