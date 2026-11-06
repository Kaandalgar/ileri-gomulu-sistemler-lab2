#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_0
#define LED_PIN GPIO_NUM_4
#define DEBOUNCE_US 50000LL

static QueueHandle_t olay_kuyrugu;
static volatile uint32_t kayip_olay = 0;

static void buton_kesmesi(void *arg)
{
    (void)arg;

    int64_t zaman_us = esp_timer_get_time();
    BaseType_t gorev_uyansin = pdFALSE;

    if (xQueueSendFromISR(olay_kuyrugu,
                          &zaman_us,
                          &gorev_uyansin) != pdTRUE)
    {
        kayip_olay++;
    }

    if (gorev_uyansin)
    {
        portYIELD_FROM_ISR();
    }
}

void app_main(void)
{
    olay_kuyrugu = xQueueCreate(64, sizeof(int64_t));

    if (olay_kuyrugu == NULL)
    {
        printf("Kuyruk olusturulamadi.\n");
        return;
    }

    gpio_config_t led_ayar = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&led_ayar));
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));

    gpio_config_t buton_ayar = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE};

    ESP_ERROR_CHECK(gpio_config(&buton_ayar));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(
        gpio_isr_handler_add(BUTTON_PIN, buton_kesmesi, NULL));

    uint32_t ham_sayac = 0;
    uint32_t kabul_sayac = 0;
    int led_durumu = 0;

    int64_t onceki_ham = 0;
    int64_t son_kabul = 0;
    int64_t zaman_us;

    printf("\nLAB 2 - BOLUM 3: BUTON KESMESI\n");
    printf("Debounce: 50 ms\n");
    printf("Hazir. BOOT dugmesine basin.\n");
    printf("ham_no,zaman_us,ham_aralik_us,kabul,kabul_toplam,kayip\n");

    while (1)
    {
        if (xQueueReceive(olay_kuyrugu,
                          &zaman_us,
                          portMAX_DELAY) == pdTRUE)
        {

            int64_t aralik_us =
                (ham_sayac == 0) ? 0 : zaman_us - onceki_ham;

            onceki_ham = zaman_us;
            ham_sayac++;

            int kabul = (kabul_sayac == 0) ||
                        (zaman_us - son_kabul >= DEBOUNCE_US);

            if (kabul)
            {
                son_kabul = zaman_us;
                kabul_sayac++;

                led_durumu = !led_durumu;
                ESP_ERROR_CHECK(
                    gpio_set_level(LED_PIN, led_durumu));
            }

            printf("%" PRIu32 ",%" PRId64 ",%" PRId64
                   ",%d,%" PRIu32 ",%" PRIu32 "\n",
                   ham_sayac,
                   zaman_us,
                   aralik_us,
                   kabul,
                   kabul_sayac,
                   kayip_olay);
        }
    }
}