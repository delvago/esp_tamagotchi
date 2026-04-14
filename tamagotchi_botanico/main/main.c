#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "BOTANICO_ADC";

void app_main(void)
{
    // Configuración del ADC
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    // Es buena práctica verificar el retorno de las funciones de configuración
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // Configurar el canal 
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Resolución de 12 bits
        .atten = ADC_ATTEN_DB_12, // Rango Completo de voltaje (0-3.3V)
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_9, &config);

    ESP_LOGI(TAG, "Cerebro iniciado. ADC configurado en GPIO10.");

    int lectura_raw = 0;

    while (1) {
        // Realizar la lectura
        esp_err_t ret = adc_oneshot_read(adc1_handle, ADC_CHANNEL_9, &lectura_raw);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Humedad Raw: %d", lectura_raw);
        } else {
            ESP_LOGE(TAG, "Error al leer ADC: %s", esp_err_to_name(ret));
        }

        // Pausa de 5 segundos
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
