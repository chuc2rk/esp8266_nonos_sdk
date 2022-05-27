/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "ets_sys.h"

#define LED_PIN 12
#define BUTTON_PIN 13

static bool ledState = false;

void buttonInteruptHandle()
{
	ETS_GPIO_INTR_DISABLE(); //Vô hiệu hóa ngắt trên các chân
	gpio_pin_intr_state_set(BUTTON_PIN, GPIO_PIN_INTR_DISABLE);
	os_delay_us(5000);

	uint32 gpioStatus = GPIO_REG_READ(GPIO_STATUS_ADDRESS); //đọc trạng thái ngắt GPIO
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpioStatus); //xóa cờ ngắt

	if (gpioStatus & BIT(BUTTON_PIN)){ //Nếu thực sự có ngắt xảy ra ở nút nhấn
		if (!GPIO_INPUT_GET(BUTTON_PIN)){
			ledState = !ledState;
			GPIO_OUTPUT_SET(LED_PIN, ledState);
			os_printf("LED state: %d \n", ledState);
		}
	}

	gpio_pin_intr_state_set(BUTTON_PIN, GPIO_PIN_INTR_NEGEDGE);
	ETS_GPIO_INTR_ENABLE();// Kích hoạt lại ngắt sau khi đã disable ở trên
}


void ICACHE_FLASH_ATTR
user_init(void)
{
	uart_init(115200,115200);
	os_printf("\n=======================================\n");
    os_printf("\t\t SDK version: %s \n", system_get_sdk_version());
    os_printf("\n=======================================\n");

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); //chức năng GPIO12 cho MTDI
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);//chức năng GPIO13 cho MTCK
    GPIO_OUTPUT_SET(LED_PIN, 0); //set output cho GPIO12
    GPIO_DIS_OUTPUT(BUTTON_PIN); //set input cho GPIO13
    PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U);//set pullup cho GPIO13

    ETS_GPIO_INTR_DISABLE(); //Vô hiệu hóa ngắt trên các chân
    ETS_GPIO_INTR_ATTACH(buttonInteruptHandle, NULL); //hàm được gọi khi có ngắt
    gpio_pin_intr_state_set(BUTTON_PIN, GPIO_PIN_INTR_NEGEDGE); //ngắt cạnh xuống
    ETS_GPIO_INTR_ENABLE(); //Kích hoạt ngắt
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

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}


