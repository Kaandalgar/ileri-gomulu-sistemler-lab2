#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"

#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_io_reg.h"

#define LED_MASK (1UL << 4)

static void registerlari_yazdir(const char *baslik)
{
    printf("\n--- %s ---\n", baslik);

    printf("IO_MUX GPIO4 : 0x%08" PRIx32 "\n",
           (uint32_t)REG_READ(PERIPHS_IO_MUX_GPIO4_U));

    printf("RTC PAD0     : 0x%08" PRIx32 "\n",
           (uint32_t)REG_READ(RTC_IO_TOUCH_PAD0_REG));

    printf("GPIO PIN4    : 0x%08" PRIx32 "\n",
           (uint32_t)REG_READ(GPIO_PIN4_REG));

    printf("GPIO OUT SEL : 0x%08" PRIx32 "\n",
           (uint32_t)REG_READ(GPIO_FUNC4_OUT_SEL_CFG_REG));

    printf("GPIO4 ENABLE : %" PRIu32 "\n",
           (uint32_t)((REG_READ(GPIO_ENABLE_REG) >> 4) & 1U));

    printf("GPIO4 OUT    : %" PRIu32 "\n",
           (uint32_t)((REG_READ(GPIO_OUT_REG) >> 4) & 1U));
}

static void gpio4_register_ile_ayarla(void)
{
    /* Ayarlarken cikisi devre disi birak; cikis degerini sifirla. */
    REG_WRITE(GPIO_ENABLE_W1TC_REG, LED_MASK);
    REG_WRITE(GPIO_OUT_W1TC_REG, LED_MASK);

    /* GPIO4'u RTC islevinden ayir ve RTC pull direnclerini kapat. */
    REG_CLR_BIT(RTC_IO_TOUCH_PAD0_REG,
                RTC_IO_TOUCH_PAD0_MUX_SEL |
                    RTC_IO_TOUCH_PAD0_RUE |
                    RTC_IO_TOUCH_PAD0_RDE);

    /* Standart ESP32'de GPIO islevinin numarasi 2'dir. */
    REG_SET_FIELD(PERIPHS_IO_MUX_GPIO4_U, MCU_SEL, PIN_FUNC_GPIO);

    /* Giris ve dahili pull direnclerini kapat. */
    REG_CLR_BIT(PERIPHS_IO_MUX_GPIO4_U,
                FUN_IE | FUN_PU | FUN_PD);

    /* Push-pull cikis; kesmeler kapali. */
    REG_CLR_BIT(GPIO_PIN4_REG,
                GPIO_PIN4_PAD_DRIVER |
                    GPIO_PIN4_INT_TYPE_M |
                    GPIO_PIN4_INT_ENA_M);

    /* Cikis kaynagi GPIO_OUT; cikis izni GPIO_ENABLE tarafindan. */
    REG_WRITE(GPIO_FUNC4_OUT_SEL_CFG_REG,
              SIG_GPIO_OUT_IDX | GPIO_FUNC4_OEN_SEL);

    REG_WRITE(GPIO_ENABLE_W1TS_REG, LED_MASK);
}

void app_main(void)
{
    gpio4_register_ile_ayarla();
    registerlari_yazdir("REGISTER ILE AYARLAMA");

    for (int i = 1; i <= 5; i++)
    {
        REG_WRITE(GPIO_OUT_W1TS_REG, LED_MASK);
        printf("%d: LED yandi\n", i);
        vTaskDelay(pdMS_TO_TICKS(500));

        REG_WRITE(GPIO_OUT_W1TC_REG, LED_MASK);
        printf("%d: LED sondu\n", i);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* Register ayarlarindan sonra surucu ayarlarini karsilastir. */
    gpio_config_t ayar = {
        .pin_bit_mask = LED_MASK,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&ayar));
    registerlari_yazdir("GPIO_CONFIG SONRASI");

    printf("\nLab 2 - Bolum 1 tamamlandi.\n");
}