/*! @mainpage Prueba para PPG
 *
 * @section genDesc General Description
 * En este proyecto se prueba el funcionamiento de levantar una señal de PPG
 * con un sensor max30102 (o similar).
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
 * | 06/06/2025 | Document creation		                         |
 *
 * @author Andres Venialgo (andres.venialgo@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "iir_filter.h"
#include "max3010x.h"
#include "led.h"
#include "timer_mcu.h"
#include "iir_filter.h"
#include "ble_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define BUFFER_SIZE 256
#define SAMPLE_FREQ	100
#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1

TaskHandle_t task_handle_ppg = NULL; 

/*==================[internal data definition]===============================*/
float dato_filt;
float dato;

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
/*==================[internal functions declaration]=========================*/

void processAndSend(void *pvParamenter){
	char msg[60];
	char msg_aux[60];
	float red_val;
	while(1){
        uint8_t i;
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	    for (i = 25; i < 100; i++)
	    {
		    redBuffer[i - 25] = redBuffer[i];
    	}

	    for ( i = 75; i < 100; i++)
	    {
		    while (MAX3010X_available() == false)
				MAX3010X_check(); 
            
		    redBuffer[i] = MAX3010X_getRed();
		    MAX3010X_nextSample(); 

            //red_val = (float)redValue;
	     	dato = (float)redBuffer[i];
			HiPassFilter(&dato, &dato_filt, 1);
			strcpy(msg_aux, "");
			sprintf(msg_aux, "G%.2f", dato_filt);
			strcat(msg, msg_aux);
            BleSendString(msg);
	    } 
    }
}
void FuncTimerSenial(void* param){
	xTaskNotifyGive(task_handle_ppg);
}

void initializePeripherals(){
HiPassInit(SAMPLE_FREQ, 1, ORDER_4);
    LedsInit();
	LowPassInit(SAMPLE_FREQ, 30, ORDER_2);
	HiPassInit(SAMPLE_FREQ, 1, ORDER_2);
    MAX3010X_begin();
	MAX3010X_setup( 30, 1 , 2, SAMPLE_FREQ, 69, 4096);

	ble_config_t ble_configuration = {
		"ESP_WAR_MACHINE)",
		BLE_NO_INT };
	BleInit(&ble_configuration);

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL};
	AnalogInputInit(&analog_input);
	AnalogOutputInit();

	timer_config_t timer_senial = {
	.timer = TIMER_B,
	.period = 4000, //TO DO:CONSULTAR
	.func_p = FuncTimerSenial,
	.param_p = NULL
	};
	
	TimerInit(&timer_senial);
    TimerStart(timer_senial.timer);

}
/*==================[external functions definition]==========================*/
void app_main(void){
	float red_val;
	uint32_t redValue;
	initializePeripherals();

	xTaskCreate(processAndSend, "leo y envio datos", 2048, NULL, 5, &task_handle_ppg);

	while(1){
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch(BleStatus()){
            case BLE_OFF:
                LedOff(LED_BT);
            break;
            case BLE_DISCONNECTED:
                LedToggle(LED_BT);
            break;
            case BLE_CONNECTED:
                LedOn(LED_BT);
            break;
        }
	}
}