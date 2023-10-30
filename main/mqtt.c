#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

#include "mqtt.h"

#define TAG "MQTT"
#define ROOM "/topic/qos1"
#define ROOM2 "/topic/davidson_custom_topiq"

extern SemaphoreHandle_t MQTTConnectionSemaphore;
esp_mqtt_client_handle_t client;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xSemaphoreGive(MQTTConnectionSemaphore);

        // msg_id = esp_mqtt_client_publish(client, ROOM, "data_3", 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successfull, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, ROOM2, 0);
        // ESP_LOGI(TAG, "sent subscribe successfull, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, ROOM, 1);
        // ESP_LOGI(TAG, "sent subscribe successfull, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, ROOM);
        // ESP_LOGI(TAG, "sent unsubscribe successfull, msg_id=%d", msg_id);
        break;
    
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, ms_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, ROOM2, "data", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successfull, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, ms_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, ms_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVEcNT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id: %d", event->event_id);
        break;
    }

    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%li", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_start(){
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = "mqtt://test.mosquitto.org",
    };
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

}

void mqtt_send_message(char * topic, char * message){
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "Message sent, ID: %d", message_id);
}
