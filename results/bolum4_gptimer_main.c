#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "esp_err.h"
#include "esp_timer.h"

#define ORNEK_SAYISI 1000

static int64_t zamanlar[ORNEK_SAYISI + 1];
static unsigned int sayac = 0;
static TaskHandle_t ana_gorev;

static bool alarm_callback(gptimer_handle_t timer,
                           const gptimer_alarm_event_data_t *olay,
                           void *kullanici)
{
    (void)timer;
    (void)olay;
    (void)kullanici;

    BaseType_t gorev_uyansin = pdFALSE;

    if (sayac < ORNEK_SAYISI + 1)
    {
        zamanlar[sayac] = esp_timer_get_time();
        sayac++;

        if (sayac == ORNEK_SAYISI + 1)
        {
            vTaskNotifyGiveFromISR(ana_gorev, &gorev_uyansin);
        }
    }

    return gorev_uyansin == pdTRUE;
}

void app_main(void)
{
    ana_gorev = xTaskGetCurrentTaskHandle();

    gptimer_handle_t timer = NULL;

    gptimer_config_t timer_ayar = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000};

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_ayar, &timer));

    gptimer_event_callbacks_t callbacks = {
        .on_alarm = alarm_callback};

    ESP_ERROR_CHECK(
        gptimer_register_event_callbacks(timer, &callbacks, NULL));

    gptimer_alarm_config_t alarm_ayar = {
        .alarm_count = 1000,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true};

    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_ayar));
    ESP_ERROR_CHECK(gptimer_enable(timer));

    printf("\nLAB 2 - BOLUM 4: GPTIMER\n");
    printf("Wi-Fi: baslatilmadi\n");
    printf("Hedef periyot: 1000 us\n");
    printf("Olcum basliyor...\n");
    fflush(stdout);

    ESP_ERROR_CHECK(gptimer_start(timer));

    uint32_t bildirim =
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000));

    ESP_ERROR_CHECK(gptimer_stop(timer));
    ESP_ERROR_CHECK(gptimer_disable(timer));
    ESP_ERROR_CHECK(gptimer_del_timer(timer));

    if (bildirim == 0)
    {
        printf("HATA: Olcum zaman asimina ugradi.\n");
        return;
    }

    int64_t minimum = INT64_MAX;
    int64_t maksimum = INT64_MIN;
    int64_t toplam = 0;

    printf("\nyontem,wifi,ornek,periyot_us\n");

    for (int i = 1; i <= ORNEK_SAYISI; i++)
    {
        int64_t fark = zamanlar[i] - zamanlar[i - 1];

        if (fark < minimum)
        {
            minimum = fark;
        }

        if (fark > maksimum)
        {
            maksimum = fark;
        }

        toplam += fark;

        printf("GPTimer,OFF,%d,%" PRId64 "\n", i, fark);
    }

    printf("\n--- OLCUM OZETI ---\n");
    printf("Yontem: GPTimer\n");
    printf("Wi-Fi: OFF\n");
    printf("Ornek sayisi: %d\n", ORNEK_SAYISI);
    printf("Minimum periyot: %" PRId64 " us\n", minimum);
    printf("Maksimum periyot: %" PRId64 " us\n", maksimum);
    printf("Ortalama periyot: %.3f us\n",
           (double)toplam / ORNEK_SAYISI);
    printf("Tepe-tepe degisim: %" PRId64 " us\n",
           maksimum - minimum);
    printf("Olcum tamamlandi.\n");
}