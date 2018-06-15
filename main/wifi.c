#include "esp_err.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "soc.h"

#include "lwip/sockets.h"

#include "include/wifi.h"

FILE * init_network(){
	wifi_signal = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	wifi_config_t wifi_config = {
			.sta = {
					.ssid = CONFIG_WIFI_SSID,
					.password = CONFIG_WIFI_PASSWORD
			},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	while(xEventGroupWaitBits( wifi_signal, BIT0, pdFALSE, pdTRUE, 1000 ) == 0);

	int socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
}

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
	case SYSTEM_EVENT_STA_START:
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_signal, BIT0);
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(wifi_signal, BIT0);
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		xEventGroupClearBits(wifi_signal, BIT0);
		break;
	default:
		break;
	}
	return ESP_OK;
}

void close_network() {

}
