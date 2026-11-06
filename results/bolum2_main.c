#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"

#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"

#define LED_PIN GPIO_NUM_4
#define LED_MASK (1UL << 4)
#define PERIYOT_SAYISI 100000

/* Fonksiyon isimleri objdump incelemesinde korunur. */
__attribute__((noinline)) static void test_api(void)
{
    for (int i = 0; i < PERIYOT_SAYISI; i++)
    {
        gpio_set_level(LED_PIN, 1);
        gpio_set_level(LED_PIN, 0);
    }
}

__attribute__((noinline)) static void test_yapi(void)
{
    for (int i = 0; i < PERIYOT_SAYISI; i++)
    {
        GPIO.out_w1ts = LED_MASK;
        GPIO.out_w1tc = LED_MASK;
    }
}

__attribute__((noinline)) static void test_register(void)
{
    for (int i = 0; i < PERIYOT_SAYISI; i++)
    {
        REG_WRITE(GPIO_OUT_W1TS_REG, LED_MASK);
        REG_WRITE(GPIO_OUT_W1TC_REG, LED_MASK);
    }
}

static void olcum_yap(const char *yontem,
                      void (*test_fonksiyonu)(void),
                      int tekrar)
{
    int64_t baslangic = esp_timer_get_time();

    test_fonksiyonu();

    int64_t sure_us = esp_timer_get_time() - baslangic;

    double frekans_hz =
        (double)PERIYOT_SAYISI * 1000000.0 / (double)sure_us;

    printf("%s,%d,%d,%" PRId64 ",%.2f\n",
           yontem,
           tekrar,
           PERIYOT_SAYISI,
           sure_us,
           frekans_hz);

    vTaskDelay(pdMS_TO_TICKS(100));
}

void app_main(void)
{
    gpio_config_t ayar = {
        .pin_bit_mask = LED_MASK,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&ayar));
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));

    printf("\nLAB 2 - BOLUM 2: GPIO HIZ KARSILASTIRMASI\n");
    printf("Her periyot: bir HIGH ve bir LOW yazma\n");
    printf("yontem,tekrar,periyot_sayisi,sure_us,frekans_hz\n");

    for (int tekrar = 1; tekrar <= 3; tekrar++)
    {
        olcum_yap("gpio_set_level", test_api, tekrar);
        olcum_yap("GPIO_yapisi", test_yapi, tekrar);
        olcum_yap("REG_WRITE", test_register, tekrar);
    }

    printf("Lab 2 - Bolum 2 olcumleri tamamlandi.\n");
}