#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("Iniciando el cerebro del Tamagotchi...\n");

    while (1) {
        // Aquí integraremos la lectura del sensor
        printf("Sistema en línea.\n");
        
        // Pausa de 1000 milisegundos (1 segundo) para ceder control al RTOS
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}