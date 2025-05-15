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
 * | 15/05/2025 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mecu.h"
#include "timer_mcu.h" 
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
#define BUFFER_SIZE 231
uint16_t poteValue = 0;
analog_input_config_t poteInput;
TaskHandle_t pote_task = NULL;
TaskHandle_t output_value_task = NULL;
TaskHandle_t input_value_task = NULL;
/*==================[internal data definition]===============================*/
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
void readPoteValue(void *pParameter){
	while(true){
		AnalogInputReadSingle(CH0, &poteValue);
		UartSendString(UART_PC, (char*)UartItoa(poteValue,10));
		UartSendString(UART_PC, " \r\n");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void inputEcgValue (void *pParameter){
	while(true){
		for(int i = 0; i < BUFFER_SIZE; i++){
			AnalogOutputWriteSingle(CH0, ecg[i]);
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			}
	}
}
void readPoteTask (void *pParameter){
	vTaskNotifyGiveIndexedFromISR(pote_task, pdFALSE);
}
void outputValueTask (void *pParameter){
	vTaskNotifyGiveIndexedFromISR(output_value_task, pdFALSE);
}
void inputEcgTask (void *pParameter){
	vTaskNotifyGiveIndexedFromISR(input_value_task, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void){
	timer_config_t timer_output = {
        .timer = TIMER_A,
        .period = 4000,
        .func_p = readPoteTask,
        .param_p = NULL
    };
    timer_config_t timer_input = {
        .timer = TIMER_B,
        .period = 2000,
        .func_p = outputValueTask,
        .param_p = NULL
    };	
	serial_config_t uart_config = {
		.baud_rate = 115200,
		.port = UART_PC,
		.func_p = uartKey, 
		.param_p = NULL
   };
   UartInit(&uart_config);
   TimerInit(&timer_output);
   TimerInit(&timer_input);
   AnalogInputInit(&poteInput);
   AnalogOutputInit();
   xTaskCreate(readPoteValue, "readPoteValue", 2048, NULL, 1, &pote_task);
   xTaskCreate(inputEcgValue, "inputEcgValue", 2048, NULL, 1, &output_value_task);
   xTaskCreate(outputValueTask, "outputValueTask", 2048, NULL, 1, &output_value_task);
}
/*==================[end of file]============================================*/