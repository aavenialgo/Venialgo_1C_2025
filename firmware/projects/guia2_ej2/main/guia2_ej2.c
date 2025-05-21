/*! @mainpage Proyecto 2 ejercicio 2
 *
 * @section genDesc General Description
 *
 * Cree un nuevo proyecto en el que modifique la actividad del 
 * punto 1 de manera de utilizar interrupciones para el control 
 * de las teclas y el control de tiempos (Timers). 
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
 * | 17/04/2025 | Document creation		                         |
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
#include "timer_mcu.h" 
/*==================[macros and definitions]=================================*/
#define CONFIG_MEASURE_PERIOD 1000
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
 * @brief Funcion asociada a la interrupcion del switch 1.
 * 
 * @param pParameter Puntero a parametros (no utilizado).
 */
static void functionKey1(void* param){
    measuring = !measuring;
}
/**
 * @brief Funcion asociada a la interrupcion del switch 2.
 * 
 * @param pParameter Puntero a parametros (no utilizado).
 */
static void functionKey2(void* param){
    hold = !hold;
}
/**
 * @brief Tarea que mide la distancia usando el sensor HC_SR04 y lo almacena
 * en la varible distance en centimetros.
 */
static void measureDistanceTask(void *pParameter){
    while(true){
        if(measuring){
            distance = HcSr04ReadDistanceInCentimeters();
           // printf("Distance: %d cm\n", distance);
        }
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Espera a que se notifique
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
    
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
/**
 * @brief Inicializa los perifericos utilizados en el proyecto.
 */
void inicializePeripherals(){
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2); 
    SwitchesInit();
    SwitchActivInt(SWITCH_1, functionKey1, NULL);
    SwitchActivInt(SWITCH_2, functionKey2, NULL);
    LedsInit();
}
/**
 * @brief Notifica a la tarea de medir distancia que debe ejecutarse.
 */
void functionMeasure(void* param){
  vTaskNotifyGiveFromISR(measure_task, pdFALSE);
}
/**
 * @brief Notifica a la tarea de mostrar distancia que debe ejecutarse.
 */
void functionDisplay(void* param){
  vTaskNotifyGiveFromISR(display_task, pdFALSE); 
}
/**
 * @brief Notifica a la tarea de lectura de teclas que debe ejecutarse.
 */
void functionKey(void* param){
   vTaskNotifyGiveFromISR(readKey_task, pdFALSE); 
 }
/*==================[external functions definition]==========================*/
void app_main(void){
    timer_config_t timer_measure = {
        .timer = TIMER_A,
        .period = 100000,
        .func_p = functionMeasure,
        .param_p = NULL
    };
    timer_config_t timer_display = {
        .timer = TIMER_B,
        .period = 1000000,
        .func_p = functionDisplay,
        .param_p = NULL
    };

    TimerInit(&timer_measure);
    TimerInit(&timer_display);
    
    inicializePeripherals();

    xTaskCreate(&measureDistanceTask, "Measure Distance", 2048, NULL, 5, &measure_task);
   // xTaskCreate(&readKeyTask, "Read Key", 2048, NULL, 5, &readKey_task);
    xTaskCreate(&showDistanceTask, "Show Distance", 512, NULL, 5, &display_task);

    TimerStart(timer_measure.timer);
    TimerStart(timer_display.timer);

}
/*==================[end of file]============================================*/