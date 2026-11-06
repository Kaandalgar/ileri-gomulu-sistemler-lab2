#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_timer.h"

#define ORNEK_SAYISI 1000

static int64_t zamanlar[ORNEK_SAYISI + 1];
static unsigned int sayac = 0;
static TaskHandle_t ana_gorev;

static void timer_callback(void *arg)
{
    (void)arg;

    if (sayac < ORNEK_SAYISI + 1)
    {
        zamanlar[sayac] = esp_timer_get_time();
        sayac++;

        if (sayac == ORNEK_SAYISI + 1)
        {
            xTaskNotifyGive(ana_gorev);
        }
    }
}

void app_main(void)
{
    ana_gorev = xTaskGetCurrentTaskHandle();

    esp_timer_handle_t timer = NULL;

    esp_timer_create_args_t timer_ayar = {
        .callback = timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lab2_timer",
        .skip_unhandled_events = false};

    ESP_ERROR_CHECK(esp_timer_create(&timer_ayar, &timer));

    printf("\nLAB 2 - BOLUM 4: ESP_TIMER\n");
    printf("Callback: ESP_TIMER_TASK\n");
    printf("Wi-Fi: baslatilmadi\n");
    printf("Hedef periyot: 1000 us\n");
    printf("Olcum basliyor...\n");
    fflush(stdout);

    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000));

    uint32_t bildirim =
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000));

    ESP_ERROR_CHECK(esp_timer_stop(timer));
    ESP_ERROR_CHECK(esp_timer_delete(timer));

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

        printf("esp_timer_TASK,OFF,%d,%" PRId64 "\n",
               i, fark);
    }

    printf("\n--- OLCUM OZETI ---\n");
    printf("Yontem: esp_timer (TASK)\n");
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