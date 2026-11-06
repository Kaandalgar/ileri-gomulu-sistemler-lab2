#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "wifi_secrets.h"

#define WIFI_BAGLANDI BIT0
#define WIFI_BASARISIZ BIT1

static EventGroupHandle_t wifi_olaylari;
static int wifi_tekrar = 0;

static void wifi_olay(void *arg, esp_event_base_t kaynak,
                      int32_t olay, void *veri)
{
    (void)arg;
    (void)veri;

    if (kaynak == WIFI_EVENT && olay == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (kaynak == WIFI_EVENT &&
             olay == WIFI_EVENT_STA_DISCONNECTED)
    {

        xEventGroupClearBits(wifi_olaylari, WIFI_BAGLANDI);

        if (wifi_tekrar < 3)
        {
            wifi_tekrar++;
            esp_wifi_connect();
        }
        else
        {
            xEventGroupSetBits(wifi_olaylari, WIFI_BASARISIZ);
        }
    }
    else if (kaynak == IP_EVENT &&
             olay == IP_EVENT_STA_GOT_IP)
    {

        wifi_tekrar = 0;
        xEventGroupSetBits(wifi_olaylari, WIFI_BAGLANDI);
    }
}

static bool wifi_baslat(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_olaylari = xEventGroupCreate();
    if (wifi_olaylari == NULL)
    {
        return false;
    }

    wifi_init_config_t baslangic = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&baslangic));

    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_olay, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_olay, NULL));

    wifi_config_t ayar = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD}};

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &ayar));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    printf("Wi-Fi baglantisi bekleniyor...\n");

    EventBits_t sonuc = xEventGroupWaitBits(
        wifi_olaylari,
        WIFI_BAGLANDI | WIFI_BASARISIZ,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(30000));

    if (!(sonuc & WIFI_BAGLANDI))
    {
        printf("Wi-Fi baglanamadi. Olcum yapilmadi.\n");
        return false;
    }

    printf("Wi-Fi baglandi. Guc tasarrufu kapali.\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    return true;
}