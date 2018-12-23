#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "Mqtt.h"
#include "Json_parse.h"
#include "Smartconfig.h"
#include "E2prom.h"
#include "Led.h"

#define MQTT_JSON_TYPE  0X03

static const char *TAG = "MQTT";
extern const int CONNECTED_BIT;

esp_mqtt_client_handle_t client;
EventGroupHandle_t mqtt_event_group;
static const int MQTT_CONNECTED_BIT = BIT0;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    char topic_subscribe[10]="$creq/#"; //订阅平台指令
    //char topic_publish[10]="$dp"; //发布平台指令

    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);

            msg_id = esp_mqtt_client_subscribe(client, topic_subscribe, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            
            //msg_id = esp_mqtt_client_publish(client, "$dp", mqtt_send1, 36, 0, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);          
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGW(TAG, "MQTT_EVENT_DATA=%s",event->data);

            parse_objects_mqtt(event->data);//收到平台MQTT数据并解析
            bzero(event->data,strlen(event->data));
            Mqtt_Send_Msg();//收到控制指令后，回复一个数据，更新状态
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

void Mqtt_Send_Msg(void)
{
    int msg_id;
    int i=0;
    char mqtt_send[100];

    creat_json *pCreat_json=malloc(sizeof(creat_json));
    create_mqtt_json(pCreat_json);

    bzero(mqtt_send,sizeof(mqtt_send));
    //前三个固定head 第一个字节为数据类型 第二、三个字节为数据长度
    mqtt_send[0]=MQTT_JSON_TYPE;
    mqtt_send[1]=0X00;
    mqtt_send[2]=pCreat_json->creat_json_c;
    //从第四个字节开始为数据
    for(i=0;i<pCreat_json->creat_json_c;i++)
    {
        mqtt_send[3+i]=pCreat_json->creat_json_b[i];
    }
    mqtt_send[3+i]='\0';

    /*printf("mqtt_send hex=\n");
    for(int j=0;j<(pCreat_json->creat_json_c+3);j++)
    {
        printf("%x ",mqtt_send[j]);
    }
    printf("\n"); */

    free(pCreat_json);

    msg_id = esp_mqtt_client_publish(client, "$dp", mqtt_send, pCreat_json->creat_json_c+3, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    Led_Status=LED_STA_SENDDATA;
}

static void MqttSend_Task(void* arg)
{

    while(1)
    {
        xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT , false, true, portMAX_DELAY); 
        xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT , false, true, portMAX_DELAY); 
        Mqtt_Send_Msg();
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
    
}

void initialise_mqtt(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://mqtt.heclouds.com",
        .port=6002,
        .event_handle = mqtt_event_handler,
        .username = ProductId,
        .password = SerialNum,
        .client_id = Device_id
    };

    

    mqtt_event_group = xEventGroupCreate();

    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT , false, true, portMAX_DELAY); 
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    xTaskCreate(MqttSend_Task, "MqttSend_Task", 4096, NULL, 5, NULL);
}
