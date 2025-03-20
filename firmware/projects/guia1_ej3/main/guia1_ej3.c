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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/
typedef struct leds
{
      uint8_t mode;     //  ON, OFF, TOGGLE
	uint8_t n_led;      //  indica el número de led a controlar
	uint8_t n_ciclos;   //  indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   //  indica el tiempo de cada ciclo
} my_leds; 


/*==================[internal data definition]===============================*/


/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/