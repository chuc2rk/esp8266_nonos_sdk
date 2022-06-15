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
#include "pwm.h"


static os_timer_t pwm_timer; //khởi tạo soft timer cho pwm
uint32 io_info [1][3] = {PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12, 12};//GPIO12 cho channel 1
uint32 period = 1000; // chu kỳ 1000us
int32 duty[1] = {0};//khai báo duty cho 1 channel
int32 max_duty = 1000*1000/45; //công thức tính max_duty = period*1000/45
uint32 pwm_channel_num = 1; //chỉ dùng 1 channel pwm
int32 percent = 10; //phần trăm tỷ lệ tăng hoặc giảm độ sáng
int incre_or_decre = 1; // 1 tăng, -1 giảm

void pwm_cb(void)
{
	pwm_set_duty(duty[0], 0); // cài đặt giá trị duty cho channel 0
	pwm_start();
	duty[0] = duty[0] + (max_duty*percent/100)* incre_or_decre;

	if (duty[0] > max_duty) {
		duty[0] = max_duty;
		incre_or_decre = -1;   // giảm duty xuống
	}

	if (duty[0] < 0) {
		duty[0] = 0;
		incre_or_decre = 1;   // tăng duty lên
	}
}


void ICACHE_FLASH_ATTR
user_init(void)
{
	uart_init(115200,115200);
	os_printf("\n=======================================\n");
    os_printf("\t\t SDK version: %s \n", system_get_sdk_version());
    os_printf("\n=======================================\n");

    pwm_init(period, duty, pwm_channel_num, io_info); //khởi tạo PWM
    pwm_start();

    os_timer_disarm(&pwm_timer); //Stop pwm_timer
    os_timer_setfn(&pwm_timer, (os_timer_func_t *)pwm_cb, NULL);//cài đặt hàm được gọi khi pwm_timer đếm xong
    os_timer_arm(&pwm_timer, 250, true);//kích hoạt led_timer với 500ms, true có nghĩa là lặp đi lặp lại timer 500ms này
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


