/*
 * 1) Modificar el siguiente programa para que los leds enciendan secuencialmente.
 *
 * 2) Sacar conclusiones: cuántos semáforos debo utilizar: 1 o 3? funciona en ambos casos? por qué?
 *
 * 3) Implementar mediante un semáforo contador, que la secuencia se realice (completa) 5 veces.
 *
 */


#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#define led1_port	1
#define led1_pin	18
#define led2_port	1
#define led2_pin	21
#define led3_port	1
#define led3_pin	24



void Configuracion_uC(void){

	//####### GPIO #########

	Chip_GPIO_Init(LPC_GPIO);

	//####### LEDS #########

	Chip_IOCON_PinMux(LPC_IOCON, led1_port, led1_pin, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_IOCON_PinMux(LPC_IOCON, led2_port, led2_pin, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_IOCON_PinMux(LPC_IOCON, led3_port, led3_pin, IOCON_MODE_INACT, IOCON_FUNC0);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, led1_port, led1_pin);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, led2_port, led2_pin);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, led3_port, led3_pin);

	Chip_GPIO_SetPinState(LPC_GPIO, led1_port, led1_pin, false);
	Chip_GPIO_SetPinState(LPC_GPIO, led2_port, led2_pin, false);
	Chip_GPIO_SetPinState(LPC_GPIO, led3_port, led3_pin, false);

}



/* Tarea 1: Blinky del primer led */
static void Led1(void *pvParameters){

	while(1){

		Chip_GPIO_SetPinToggle(LPC_GPIO, led1_port, led1_pin);

		vTaskDelay(200/portTICK_RATE_MS);

		Chip_GPIO_SetPinToggle(LPC_GPIO, led1_port, led1_pin);

		vTaskDelay(200/portTICK_RATE_MS);

	}
}


/* Tarea 2: Blinky del primer led */
static void Led2(void *pvParameters){

	while(1){

		Chip_GPIO_SetPinToggle(LPC_GPIO, led2_port, led2_pin);

		vTaskDelay(200/portTICK_RATE_MS);

		Chip_GPIO_SetPinToggle(LPC_GPIO, led2_port, led2_pin);

		vTaskDelay(200/portTICK_RATE_MS);

	}
}


/* Tarea 3: Blinky del tercer led */
static void Led3(void *pvParameters){

	while(1){

		Chip_GPIO_SetPinToggle(LPC_GPIO, led3_port, led3_pin);

		vTaskDelay(200/portTICK_RATE_MS);

		Chip_GPIO_SetPinToggle(LPC_GPIO, led3_port, led3_pin);

		vTaskDelay(200/portTICK_RATE_MS);

	}
}



/****************************************************************************************************/
/**************************************** MAIN ******************************************************/
/****************************************************************************************************/

int main(void){

	/* Levanta la frecuencia del micro */
	SystemCoreClockUpdate();

	/* Configuración inicial del micro */
	Configuracion_uC();

    /* Creacion de tareas */
	xTaskCreate(Led1, (char *) "Blinky Led 1",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);
	xTaskCreate(Led2, (char *) "Blinky Led 2",
	    		configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
	    		(xTaskHandle *) NULL);
	xTaskCreate(Led3, (char *) "Blinky Led 3",
	    		configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
	    		(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

