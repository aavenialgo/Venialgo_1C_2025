/*! @mainpage Proyecto EPROM
 *
 * @section genDesc General Description
 *	Este es el proyecto integrador de la asignatura electronica programable.
 * El mismo consiste la utilizacion de dos sensores, uno de ppg y otro de 
 * biopotenciales (para levantar un ECG), los cuales se encuentran conectados a 
 * un microcontrolador ESP32.
 * El objetivo del proyecto es la lectura de los datos de ambos sensores y su
 * posterior envio a una aplicacion movil mediante bluetooth.
 * Constara de un procesamiento basico de las señales como filtro para mejorar
 * su calidad.
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
#include "uart_mcu.h"
#include "neopixel_stripe.h"

//#include "spo2_algorithm.h"
//#include <max3010x.h>

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

analog_input_config_t poteInput = {  //senial
    .input = CH2, 
};
TaskHandle_t task_handle_ppg = NULL;

float dato_filt;
float dato;
/*==================[internal functions declaration]=========================*/

static void processAndSendEcg(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	static uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH2, &valor);
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
void procesAndSendPpg(void *pvParameter)
{	 char msg[32];
    float analog_raw, analog_hp, analog_filt;
    uint16_t adcValue = 0;
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        AnalogInputReadSingle(poteInput.input, &adcValue);
        analog_raw = (float)adcValue;

        HiPassFilter(&analog_raw, &analog_hp, 1);
        LowPassFilter(&analog_hp, &analog_filt, 1);
        snprintf(msg, sizeof(msg), "%0.2f\n", analog_filt);
		printf(">PPG:%0.2f\r\n", analog_filt);		
		
		//BleSendString(msg);

		// Enviar el valor del ADC por UART
 		// UartSendString(CH1, ">ad:");
 		// UartSendString(CH1, (char*)UartItoa(adcValue,10));
 		// UartSendString(CH1, " \r\n");
		// UartSendString(CH1, ">ad:");
 		// UartSendString(CH1, (char*)UartItoa(adcValue,10));
 		// UartSendString(CH1, " \r\n");
        //BleSendString(msg);
    }
}

// void processAndSendPpg(void *pvParameter)
// {	while(true){
// 		uint16_t adcValue = 0;
// 		AnalogInputReadSingle(poteInput.input, &adcValue);
// 		UartSendString(CH1, ">ad:");
// 		UartSendString(CH1, (char*)UartItoa(adcValue,10));
// 		UartSendString(CH1, " \r\n");
// 		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
// 	}
// }

void FuncTimerSenialECG(void* param){
    xTaskNotifyGive(task_handle_ecg);
}

void FuncTimerSenialPPG(void* param){
    xTaskNotifyGive(task_handle_ppg);
}

void inicialitePeripherals(){
	LedsInit();
	LedOn(LED_3);
	LowPassInit(SAMPLE_FREQ, 20, ORDER_2);
    HiPassInit(SAMPLE_FREQ, 0.5, ORDER_2);

	ble_config_t ble_configuration = {
		"ESP_WAR_MACHINE_ppg",
		BLE_NO_INT };
	BleInit(&ble_configuration);

	timer_config_t timer_ecg = {
	.timer = TIMER_B,
	.period = T_SENIAL,
	.func_p = FuncTimerSenialECG,
	.param_p = NULL
	};

	timer_config_t timer_ppg = {
	.timer = TIMER_A,
	.period = 4000,
	.func_p = FuncTimerSenialPPG,
	.param_p = NULL
	};

	TimerInit(&timer_ppg);
	TimerStart(timer_ppg.timer);
	//TimerInit(&timer_ecg);
    //TimerStart(timer_ecg.timer);

	AnalogInputInit(&poteInput);
	AnalogOutputInit();

	serial_config_t uart_config = {
		.baud_rate = 115200,
		.port = CH1,
		.func_p = NULL, 
		.param_p = NULL
   };

   UartInit(&uart_config);
	UartSendString(CH1, ">ad: configuracion de perifericos\r\n");


}
/*==================[external functions definition]==========================*/
void app_main(void){

	inicialitePeripherals();

//	xTaskCreate(processAndSendEcg, "Leo y envio datos ECG", 2048, NULL, 5, &task_handle_ecg);
	xTaskCreate(procesAndSendPpg, "Leo y envio datos PPG", 2048, NULL, 5, &task_handle_ppg);

	// while(1){
    //     vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    //     switch(BleStatus()){
    //         case BLE_OFF:
    //             LedOff(LED_BT);
    //         break;
    //         case BLE_DISCONNECTED:
    //             LedToggle(LED_BT);
    //         break;
    //         case BLE_CONNECTED:
    //             LedOn(LED_BT);
    //         break;
    //     }
	// }
}
/*==================[end of file]============================================*/