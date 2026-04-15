#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

//  Log
static const char *TAG = "BOTANICO_ADC";

// Credenciales WiFi
#define WIFI_SSID "Margomez"
#define WIFI_PASS "3108201204"
#define MAXIMUM_RETRY 5

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "Reintentando conexión al router...");
        } else {
            ESP_LOGE(TAG, "Fallo al conectar al Wi-Fi");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "¡Conectado! IP asignada: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    } 
} 

void wifi_init_sta(void){
    // 1. Inicializar la pila de red (LwIP)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 2. Configuración por defecto del Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Registrar nuestro manejador de eventos
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    // 4. Configurar las credenciales
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Nivel de seguridad
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    // 5. Encender la antena
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Inicialización de Wi-Fi terminada.");
}


void app_main(void)
{

    // Inicializando memoria NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Iniciando conexión WiFi
    ESP_LOGI(TAG, "Iniciando conexión WiFi...");
    wifi_init_sta();

    vTaskDelay(pdMS_TO_TICKS(5000));

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
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}