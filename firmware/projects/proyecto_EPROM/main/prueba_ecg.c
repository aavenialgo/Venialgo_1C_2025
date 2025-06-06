/*! @mainpage Prueba para ecg
 *
 * @section genDesc General Description
 * Este es el proyecto es para probar el funcionamiento de la placa de biopotenciales
 * para levantar una señal de ECG.
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
 * | 22/05/2025 | Document creation		                         |
 *
 * @author Andres Venialgo
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led.h"
#include "timer_mcu.h"
#include "iir_filter.h"
#include "ble_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1
#define SAMPLE_FREQ 200
#define T_SENIAL 4000
#define CHUNK 8
#define PERIODO_REFRACTARIO 250000
#define PERIODO_MUESTREO 10000

uint8_t muestras_en_periodo_refractario = PERIODO_REFRACTARIO / PERIODO_MUESTREO;
int16_t UMBRAL_ONDA_R = 400;
int16_t UMBRAL_INF_FC = 60;
int16_t UMBRAL_SUP_FC = 80;
uint16_t valor_actual = 0;
static float ecg_filt[CHUNK] = {0};
static float ecg_muestra[CHUNK] = {0};

/*==================[internal data definition]===============================*/
bool filter = false;
TaskHandle_t task_handle_ecg = NULL;
float dato_filt;
float dato;
/*==================[internal functions declaration]=========================*/

static void processAndSend(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	static uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valor);
		ecg_muestra[i] = valor;
		valor_actual = valor;
		i++;

		if (i == CHUNK)
		{
			HiPassFilter(&ecg_muestra[0], ecg_filt, CHUNK);
			LowPassFilter(ecg_filt, ecg_filt, CHUNK);
			strcpy(msg, "");

			for (uint8_t j = 0; j < CHUNK; j++)
			{
				sprintf(msg_chunk, "*G%.2f*", ecg_filt[j]);
				strcat(msg, msg_chunk);
			}
			BleSendString(msg);
			i = 0;
		}
	}
}
void FuncTimerSenialECG(void* param){
    xTaskNotifyGive(task_handle_ecg);
}
void inicialitePeripherals(){
	LedsInit();
	LowPassInit(SAMPLE_FREQ, 30, ORDER_2);
    HiPassInit(SAMPLE_FREQ, 1, ORDER_2);
	ble_config_t ble_configuration = {
		"ESP_WAR_MACHINE)",
		BLE_NO_INT };
	BleInit(&ble_configuration);
		timer_config_t timer_senial = {
        .timer = TIMER_B,
        .period = T_SENIAL*CHUNK,
        .func_p = FuncTimerSenialECG,
        .param_p = NULL
    };
	TimerInit(&timer_senial);
    TimerStart(timer_senial.timer);

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL};

	AnalogInputInit(&analog_input);
	AnalogOutputInit();

}
/*==================[external functions definition]==========================*/
void app_main(void){

	inicialitePeripherals();
	xTaskCreate(processAndSend, "Leo y envio datos", 2048, NULL, 5, &task_handle_ecg);

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
/*==================[end of file]============================================*/