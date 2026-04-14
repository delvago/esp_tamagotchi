#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

void app_main(void)
{
    // Configuración del ADC
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    // Configurar el canal 
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Resolución de 12 bits
        .atten = ADC_ATTEN_DB_12, // Rango Completo de voltaje (0-3.3V)
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_9, &config);

    printf("Cerebro iniciado. ADC configurado en GPIO10. \n");

    int lectura_raw = 0;

    while (1) {
        // Realizar la lectura
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_9, &lectura_raw);

        printf("Humedad Raw: %d\n", lectura_raw);

        // Pausa de 5 segundos
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}