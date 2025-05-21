/*! @mainpage Template
 *
 * @section genDesc General Description
 * Cree un nuevo proyecto en el que modifique la actividad del punto 2 
 * agregando ahora el puerto serie. Envíe los datos de las mediciones 
 * para poder observarlos en un terminal en la PC, siguiendo el siguiente formato:
 * 3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + 
 * cambio de línea “ \r\n”
 * Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 * Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
 * 
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
/**
 * @brief Handle de la tarea de medicion de distancia.
 */
TaskHandle_t measure_task = NULL;
/**
 * @brief Handle de la tarea de lectura de teclas.
 */
TaskHandle_t readKey_task = NULL;
/**
 * @brief Handle de la tarea de visualizacion de distancia.
 */
TaskHandle_t display_task = NULL;
/**
 * @brief bandera que indica si se esta midiendo o no.
 */
bool measuring = true;
/**
 * @brief Variable que almacena la tecla leída.
 */
uint8_t key = 0;
/**
 * @brief Variable que indica si se esta en modo hold o no.
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
            //printf("Distance: %d cm\n", distance);
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
        if (measuring && !hold) {
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
        else{
            LcdItsE0803Off();
            LedsOffAll();
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
/**
 * @brief Tarea que lee el valor de la tecla y lo almacena en la variable key.
 *  Realiza la misma tarea que la funcion asociada a la interrupcion del switch 1 y 2.
 * Luego escribe el valor de la distancia por UART.
 * @param param Puntero a parametros (no utilizado).
 */
void uartKey(void *param){
	while(true){
		if (UartReadByte(UART_PC, &key)== 0){
			switch(key){
				case 'O': // 'O'
					measuring = !measuring;
					break;
				case 'H': // 'H'
					hold = !hold;
					break;
				default:
				key = 0;
					break;
			}
		}
        UartSendString(UART_PC, (char*)UartItoa(distance,10));
        UartSendString(UART_PC, " cm\r\n");

		vTaskDelay(10 / portTICK_PERIOD_MS);
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
 * @brief Notifica a la tarea de lectura de tecla que debe ejecutarse.
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