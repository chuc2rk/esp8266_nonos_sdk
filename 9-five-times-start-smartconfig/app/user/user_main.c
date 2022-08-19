#include "ets_sys.h"
#include "driver/uart.h"
#include "driver/key.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

#define LED_GPIO 2
#define flash_data_sector 550
#define flash_data_addr	flash_data_sector * 4096 + 20
os_timer_t check_flag_timer;


void ICACHE_FLASH_ATTR set_on_off_flag(bool is_reset)
{
	if (!is_reset) { //read first
		uint8 save_number[4];
		spi_flash_read(flash_data_addr, (uint32*)save_number, sizeof(save_number));
		if (save_number[0] > 8 || save_number[0] < 0) {
			save_number[0] = 1;
		} else {
			save_number[0]++;
		}

		spi_flash_erase_sector(flash_data_sector);
		spi_flash_write(flash_data_addr, (uint32*)save_number, sizeof(save_number));
	} else {
		uint8 save_number[4];
		save_number[0] = 0;
		spi_flash_erase_sector(flash_data_sector);
		spi_flash_write(flash_data_addr, (uint32*)save_number, sizeof(save_number));
	}
}

uint8 ICACHE_FLASH_ATTR get_on_off_flag()
{
	uint8 temp_save_data[4];
	spi_flash_read(flash_data_addr, (uint32*)temp_save_data, sizeof(temp_save_data));
	os_printf("Current temp save data: %d \n", temp_save_data);
	//if read not success
	if (temp_save_data[0] == -1) {
		temp_save_data[0] = 1;
		spi_flash_erase_sector(flash_data_sector);
		spi_flash_write(flash_data_addr, (uint32*)temp_save_data, sizeof(temp_save_data));
	}

	return temp_save_data[0];

}

void led_blink()
{
	uint8 i;
	for (i = 1; i <4; i++) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_GPIO), 1);
		os_delay_us(60000);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_GPIO), 0);
		os_delay_us(60000);
	}
}

void hw_timer_test_cb(void) {
	static uint8 status_flag = 0;
	status_flag++;

	if (status_flag == 1) {
		uint8 flag = get_on_off_flag();
		os_printf("Current save flag: %d \n", flag);
		if (flag > 4) {
			led_blink();
			set_on_off_flag(true);
			smart_config_init();
			os_timer_disarm(&check_flag_timer);
		}
	} else if (status_flag == 3) {
		set_on_off_flag(true);
	}
}

void user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); //setup GPIO12 as GPIO pin
    GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_GPIO), 0);

    set_on_off_flag(false);

    os_timer_disarm(&check_flag_timer);
    os_timer_setfn(&check_flag_timer, (os_timer_func_t *)hw_timer_test_cb, NULL);
    os_timer_arm(&check_flag_timer, 1000, true);
}



/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
