/*! @mainpage Template
 *
 * @section genDesc General Description
 * Diseñar e implementar una aplicación, basada en el driver analog io mcu.y 
 * el driver de transmisión serie uart mcu.h, que digitalice una señal analógica 
 * y la transmita a un graficador de puerto serie de la PC. Se debe tomar la 
 * entrada CH1 del conversor AD y la transmisión se debe realizar por la UART 
 * conectada al puerto serie de la PC, en un formato compatible con un graficador 
 * por puerto serie. 
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
 * @author Andres Venialgo
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h" 
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/** 
 * @brief Tamaño del buffer de datos a enviar por UART. 
*/
#define BUFFER_SIZE 231

analog_input_config_t poteInput = {
    .input = CH1, 
};
TaskHandle_t adc_task_handle = NULL;
TaskHandle_t dca_task_handle= NULL;

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
static void readAdcValueTask(void *pParameter){

	while(true){
		uint16_t adcValue = 0;

		AnalogInputReadSingle(poteInput.input, &adcValue);
		UartSendString(UART_PC, ">ad:");
		UartSendString(UART_PC, (char*)UartItoa(adcValue,10));
		UartSendString(UART_PC, " \r\n");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

static void outputDacValueTask (void *pParameter){
	while(true){
		for(int i = 0; i < BUFFER_SIZE; i++){
			AnalogOutputWrite(ecg[i]);
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			}
	}
}
void function_dca (void *pParameter){
	vTaskNotifyGiveFromISR(dca_task_handle, pdFALSE);
}
void function_adc (void *pParameter){
	vTaskNotifyGiveFromISR(adc_task_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void){
	timer_config_t timer_adc = {
        .timer = TIMER_A,
        .period = 4000,
        .func_p = function_adc,
        .param_p = NULL
    };
    timer_config_t timer_dca = {
        .timer = TIMER_B,
        .period = 2000,
        .func_p = function_dca,
        .param_p = NULL
    };	
	serial_config_t uart_config = {
		.baud_rate = 115200,
		.port = UART_PC,
		.func_p = NULL, 
		.param_p = NULL
   };

   UartInit(&uart_config);
   TimerInit(&timer_adc);
   TimerInit(&timer_dca);
   AnalogInputInit(&poteInput);
   AnalogOutputInit();

   xTaskCreate(&readAdcValueTask, "readDcaValue", 2048, NULL, 1, &adc_task_handle);
   xTaskCreate(&outputDacValueTask, "inputAdcValue", 2048, NULL, 1, &dca_task_handle);

    TimerStart(TIMER_A); 
    TimerStart(TIMER_B);
}
/*==================[end of file]============================================*/