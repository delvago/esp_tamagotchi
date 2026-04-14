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
        .atten = ADC_ATTEN_DB_12,         // Rango Completo de voltaje (0-3.3V)
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_9, &config);

    ESP_LOGI(TAG, "Cerebro iniciado. ADC configurado en GPIO10.");

    while (1) {
        // Realizar la lectura
        int suma_lecturas = 0;
        int num_muestras = 10;
        int promedio_raw = 0;
        int successful_reads = 0; // Contador para lecturas exitosas

        // Tomamos 10 lecturas
        for (int i = 0; i < num_muestras; i++) {
            int lectura_temp = 0;
            esp_err_t ret = adc_oneshot_read(adc1_handle, ADC_CHANNEL_9, &lectura_temp);
            if (ret == ESP_OK) {
                suma_lecturas += lectura_temp;
                successful_reads++;
            } else {
                ESP_LOGE(TAG, "Error al leer ADC en muestra %d: %s", i, esp_err_to_name(ret));
            }
            
            // Una micro-pausa de 10ms entre lecturas da tiempo al hardware a estabilizarse
            vTaskDelay(pdMS_TO_TICKS(10)); 
        }

        // Calculamos el valor suavizado y aplicamos la lógica de interpolación
        if (successful_reads > 0) {
            promedio_raw = suma_lecturas / successful_reads;
            
            // --- CONVERSIÓN A PORCENTAJE ---
            // 1. Normalización Invertida (Matemática con floats)
            float calculo = 100.0 - (((float)promedio_raw - 1300.0) / (3350.0 - 1300.0) * 100.0);
            int porcentaje = (int)calculo; 

            // 2. Clamping (Barreras lógicas de seguridad)
            if (porcentaje > 100) {
                porcentaje = 100;
            } else if (porcentaje < 0) {
                porcentaje = 0;
            }

            // 3. Reporte final
            ESP_LOGI(TAG, "Raw Promediado: %d | Humedad Real: %d%%", promedio_raw, porcentaje);
            
        } else {
            ESP_LOGE(TAG, "No se pudo obtener ninguna lectura de ADC para el promedio.");
        }

        // Pausa de 5 segundos entre lecturas
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}