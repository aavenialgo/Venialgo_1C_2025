/*! @mainpage Proyecto 2 ejercicio 1
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
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
#define CONFIG_MEASURE_PERIOD 1000
#define CONFIG_DISPLAY_PERIOD 1000
/*==================[internal data definition]===============================*/
TaskHandle_t measure_task = NULL;
TaskHandle_t readKey_task = NULL;
TaskHandle_t display_task = NULL;
bool measuring = true;
int8_t tecla;
bool hold = false;
uint8_t distance = 0;
/*==================[internal functions declaration]=========================*/
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
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    }
}

static void measureDistanceTask(void *pParameter){
    while(true){
        if(measuring){
            distance = HcSr04ReadDistanceInCentimeters();
            printf("Distance: %d cm\n", distance);
        }
        vTaskDelay(CONFIG_MEASURE_PERIOD / portTICK_PERIOD_MS);
    }
}

static void showDistanceTask(void *pParameter){
    while(true){
        if (measuring) {
            if (distance < 10){
                ledOff(LED_1);
                ledOff(LED_2);
                ledOff(LED_3);
            } else if (distance < 20){
                ledOn(LED_1);
                ledOn(LED_2);
                ledOff(LED_3);
            } else if (distance < 30){
                ledOn(LED_1);
                ledOn(LED_2);
                ledOff(LED_3);
            }
            else {
                ledOn(LED_1);
                ledOn(LED_2);
                ledOn(LED_3);
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
    ledInit(LED_1);
    ledInit(LED_2);
    ledInit(LED_3);
    xTaskCreate(&measureDistanceTask, "Measure Distance", 512, NULL, 5, &measure_task);
    xTaskCreate(&readKeyTask, "Read Key", 512, NULL, 5, &readKey_task);
    xTaskCreate(&showDistanceTask, "Show Distance", 512, NULL, 5, &display_task);
}
/*==================[end of file]============================================*/