/*! @mainpage Proyecto 2 ejercicio 1
 *
 * @section genDesc General Description
 *
 * 1. Diseñar un firmware que cumpla las siguientes funcionalidades:
 * a. si la distancia es menor a 10 cm, apagar todos los LEDs.
 * b. si la distancia esta entre 10, y 20 cm, enceender el LED_1.
 * c. Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * d. Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 2. Mostrar el valor de distancia en cm utilizando el display LCD.
 * 3. Usar TEC1 para activar y detener la medición.
 * 4. Usar TEC2 para mantener el resultado (“HOLD”).
 * 5. Refresco de medición: 1 s.
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral      |   ESP32 GPIO    |
 * |:------------------:|:---------------:|
 * | Ultrasonic Trigger | GPIO_3          |
 * | Ultrasonic Echo    | GPIO_2          |
 * | Button 1           | GPIO_4          |
 * | Button 2           | GPIO_15         |
 * | LED 1              | GPIO_11         |
 * | LED 2              | GPIO_10         |
 * | LED 3              | GPIO_5          |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 14/04/2025 | Document creation		                         |
 *
 * @author Andres Venialgo
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
/**  @brief periodo de refresco de la medicion en milisegundos. 
*/
#define CONFIG_MEASURE_PERIOD 1000
/**  @brief periodo de refresco de la visualizacion en milisegundos. 
*/
#define CONFIG_DISPLAY_PERIOD 1000
/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea de medición de distancia.
 */
TaskHandle_t measure_task = NULL;
/**
 * @brief Handle de la tarea de lectura de teclas.
 */
TaskHandle_t readKey_task = NULL;
/**
 * @brief Handle de la tarea de visualización de distancia.
 */
TaskHandle_t display_task = NULL;
/**
 * @brief Variable que indica si se está midiendo o no.
 */
bool measuring = true;
/**
 * @brief Variable que almacena la tecla leída.
 */
int8_t tecla = 0;
/**
 * @brief Indica si el display está en modo "hold".
 */
bool hold = false;
/**
 * @brief Variable que almacena la distancia medida.
 */
uint8_t distance = 0;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Tarea que lee el estado de los switches y cambia el estado de las variables
 * hold y measuring.
 */
static void readKeyTask(void *pParameter){
    while(true){
    
    switch( SwitchesRead() ){
        case SWITCH_1:
        measuring = !measuring;
        break;
        case SWITCH_2:
        hold = !hold;
        break;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    }
}
/**
 * @brief Tarea que mide la distancia usando el sensor HC_SR04 y lo almacen
 * en la varible distance en centimetros.
 */
static void measureDistanceTask(void *pParameter){
    while(true){
        if(measuring){
            distance = HcSr04ReadDistanceInCentimeters();
            //printf("Distance: %d cm\n", distance);
        }
        vTaskDelay(CONFIG_MEASURE_PERIOD / portTICK_PERIOD_MS);
    }
}
/**  
* @brief Tarea que lee la variable distance, enciende los leds segun corresponda y
* escribe en el display de acuerdo al estado de hold.
*/
static void showDistanceTask(void *pParameter){
    while(true){
        if (measuring) {
            if (distance < 10){
                LedOff(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            } else if (distance < 20){
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            } else if (distance < 30){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }
            else {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        }
        if(!hold){ // si hold = false, escribo en el display
            LcdItsE0803Write(distance);
        }
        vTaskDelay(CONFIG_DISPLAY_PERIOD / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2); //Ver cual conectar
    SwitchesInit();
    LedsInit();
    xTaskCreate(&measureDistanceTask, "Measure Distance", 2048, NULL, 5, &measure_task);
    xTaskCreate(&readKeyTask, "Read Key", 2048, NULL, 5, &readKey_task);
    xTaskCreate(&showDistanceTask, "Show Distance", 512, NULL, 5, &display_task);
}
/*==================[end of file]============================================*/