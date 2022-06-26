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

#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "gpio.h"
#include "osapi.h"

#define LED_GPIO 2
struct espconn user_tcp;

void ICACHE_FLASH_ATTR
esp8266_recv(void *arg, char *pData, unsigned short len)
{
	os_printf("Received data! - %s\n", pData);
	espconn_send((struct espconn *)arg, "ESP8266 received", strlen("ESP8266 Received"));

	if (len == 0 || (pData[0] != '0' && pData[0] != '1')) {
		return;
	}
	int value = pData[0];
	GPIO_OUTPUT_SET(LED_GPIO, value);
}

void ICACHE_FLASH_ATTR
esp8266_sent(void *arg)
{
	os_printf("ESP8266 sent\n");
}

void ICACHE_FLASH_ATTR
esp8266_discon(void *arg)
{
	os_printf("Connection disconnected\n");
}

void ICACHE_FLASH_ATTR
esp8266_listen(void *arg) //hàm được gọi khi kết nối thành công
{
	struct espconn *pespconn = (struct espconn *)arg;
	espconn_regist_recvcb(pespconn, esp8266_recv); //đăng ký hàm được gọi khi esp nhận dữ liệu
	espconn_regist_sentcb(pespconn, esp8266_sent); //đăng ký hàm được gọi khi esp gửi dữ liệu xong
	espconn_regist_disconcb(pespconn, esp8266_discon); //đăng ký hàm được gọi khi esp ngắt kết nối
}

void ICACHE_FLASH_ATTR
esp8266_error(void *arg, sint8 err) //hàm được gọi khi kết nối bị lỗi
{
	os_printf("Connection error, error code: %d\r\n", err);
}

void wifi_init()
{
	wifi_set_opmode(STATION_MODE);
	struct station_config station_conf;
	os_strcpy(station_conf.ssid, "VP HHV");
	os_strcpy(station_conf.password, "hhv2021#");
	wifi_station_set_config(&station_conf);
	wifi_station_connect();
}

void tcp_init()
{
	user_tcp.type = ESPCONN_TCP;
	user_tcp.state = ESPCONN_NONE;
	user_tcp.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	user_tcp.proto.tcp->local_port = 8266;
	espconn_regist_connectcb(&user_tcp, esp8266_listen); //đăng ký hàm được gọi khi kết nối TCP được thiết lập
	espconn_regist_reconcb(&user_tcp, esp8266_error); //đăng ký hàm được gọi khi kết nối bị lỗi
	espconn_accept(&user_tcp);
	espconn_regist_time(&user_tcp, 180, 0); //Đăng ký khoảng thời gian chờ của máy chủ TCP ESP8266
}

void ICACHE_FLASH_ATTR
wifi_event_cb(System_Event_t *event)
{
	switch (event->event){
	case EVENT_STAMODE_GOT_IP:
		os_printf("IP: %d.%d.%d.\n", IP2STR(&event->event_info.got_ip.ip));
		tcp_init();
		break;
	}
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    os_printf("SDK version:%s\n", system_get_sdk_version());
    uart_init(115200, 115200);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); //setup GPIO12 as GPIO pin

    wifi_init();
    wifi_set_event_handler_cb(wifi_event_cb);
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


