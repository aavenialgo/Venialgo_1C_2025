/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
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
 * | 21/04/2025 | Document creation		                         |
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
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_MEASURE_PERIOD 1000
#define CONFIG_DISPLAY_PERIOD 1000
/*==================[internal data definition]===============================*/


TaskHandle_t measure_task = NULL;
TaskHandle_t readKey_task = NULL;
TaskHandle_t display_task = NULL;
bool measuring = true;
uint8_t tecla = 0;
bool hold = false;
uint8_t distance = 0;
uint8_t key;
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
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
    }
}

static void measureDistanceTask(void *pParameter){
    while(true){
        if(measuring){
            distance = HcSr04ReadDistanceInCentimeters();
            //printf("Distance: %d cm\n", distance);
        }
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Espera a que se notifique
    }
}

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

void uartKey(void *param){
	while(true){
		if (UartReadByte(UART_PC, &tecla)== 0){
			switch(tecla){
				case 'O': // 'O'
					measuring = !measuring;
					break;
				case 'H': // 'H'
					hold = !hold;
					break;
				default:
				tecla = 0;
					break;
			}
		}
        UartSendString(UART_PC, (char*)UartItoa(distancia,10));
        UartSendString(UART_PC, " cm\r\n");

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void inicializePeripherals(){
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2); 
    SwitchesInit();
    LedsInit();

}

void functionMeasure(void* param){
  vTaskNotifyGiveFromISR(measure_task, pdFALSE); // Notifica a la tarea de medida
}
void functionDisplay(void* param){
  vTaskNotifyGiveFromISR(display_task, pdFALSE); // Notifica a la tarea de display
}
void functionKey(void* param){
   vTaskNotifyGiveFromISR(readKey_task, pdFALSE); // Notifica a la tarea de lectura de tecla
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
	serial_config_t uart_config = {
		.baud_rate = 115200,
		.port = UART_PC,
		.func_p = uartKey, 
		.param_p = NULL
   };
    UartInit(&uart_config);
    TimerInit(&timer_measure);
    TimerInit(&timer_display);
    
    inicializePeripherals();

    xTaskCreate(&measureDistanceTask, "Measure Distance", 512, NULL, 5, &measure_task);
    xTaskCreate(&readKeyTask, "Read Key", 512, NULL, 5, &readKey_task);
    xTaskCreate(&showDistanceTask, "Show Distance", 512, NULL, 5, &display_task);

    TimerStart(timer_measure.timer);
    TimerStart(timer_display.timer);

}
/*==================[end of file]============================================*/