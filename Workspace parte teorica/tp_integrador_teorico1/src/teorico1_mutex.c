/*
 * En este ejemplo se puede observar como utilizar un mutex.
 * Hasta que no se libera el mutex en una de las tareas la otra no puede utilizar el recurso.
 */


#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define TXD0_PORT 0
#define TXD0_PIN  2
#define RXD0_PORT 0
#define RXD0_PIN  3

#define TXD1_PORT 0
#define TXD1_PIN  15
#define RXD1_PORT 0
#define RXD1_PIN  16

#define TXD2_PORT 0
#define TXD2_PIN  10
#define RXD2_PORT 0
#define RXD2_PIN  11

#define TXD3_PORT 0
#define TXD3_PIN  0
#define RXD3_PORT 0
#define RXD3_PIN  1

#define led1_port	0
#define led1_pin	22


xSemaphoreHandle mutex;


void Configuracion_uC(void){

	//####### GPIO #########

	Chip_GPIO_Init(LPC_GPIO);

	//####### LEDS #########

	Chip_IOCON_PinMux(LPC_IOCON, 0, 22, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 22);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 22, false);

	//####### UART #########

	/* Asocia los pines a la función de UART */
	Chip_IOCON_PinMux(LPC_IOCON,TXD3_PORT,TXD3_PIN,IOCON_MODE_INACT,IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON,RXD3_PORT,RXD3_PIN,IOCON_MODE_INACT,IOCON_FUNC2);

	/* Setea la UART en 9600 baudios, transmisión de 8 bits + 1 stop bit */
	Chip_UART_Init(LPC_UART3);
	Chip_UART_SetBaud(LPC_UART3, 9600);
	Chip_UART_ConfigData(LPC_UART3, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));

	/* Habilita y resetea las FIFOs. Establece trigger level de 8 bytes (la interrupción se da cuando se reciben 8 bytes) */
	/* NOTA: Las FIFOs son de 16 bytes */
	Chip_UART_SetupFIFOS(LPC_UART3, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV2));

	/* Habilita la transmisión */
	Chip_UART_TXEnable(LPC_UART3);

	/* Habilita la interrupción de UART3 cuando se alcanza el trigger level */
	Chip_UART_IntEnable(LPC_UART3, UART_IER_RBRINT);
	NVIC_SetPriority(UART3_IRQn, 1);
	NVIC_EnableIRQ(UART3_IRQn);

}



static void Task1(void *pvParameters){

	uint8_t tx[] = "tecnicas";

	while(1){

		/* Aseguro el recurso UART tomando el semaforo */
		xSemaphoreTake(mutex, portMAX_DELAY);

		for(uint32_t i=0; i<sizeof(tx); i++){

			if(tx[i] != '\0'){

				/* Envio un byte a la FIFO de UART */
				Chip_UART_SendByte(LPC_UART3, tx[i]);

				/* Esta es una demora forzada para hacer mas lenta la transmision por UART */
				for(uint32_t j=1000000; j; j--);

			}

		}

		/* Libero el semaforo una vez que termine de utilizar la UART */
		xSemaphoreGive(mutex);

	}
}



static void Task2(void *pvParameters){

	uint8_t tx[] = "digitales3\n";

	while(1){

		/* Aseguro el recurso UART tomando el semaforo */
		xSemaphoreTake(mutex, portMAX_DELAY);

		for(uint32_t i=0; i<sizeof(tx); i++){

			if(tx[i] != '\0'){

				/* Envio un byte a la FIFO de UART */
				Chip_UART_SendByte(LPC_UART3, tx[i]);

				/* Esta es una demora forzada para hacer mas lenta la transmision por UART */
				for(uint32_t j=1000000; j; j--);

			}

		}

		/* Libero el semaforo una vez que termine de utilizar la UART */
		xSemaphoreGive(mutex);

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

	/* Creacion del semaforo mutex */
	mutex = xSemaphoreCreateMutex();

    /* Creacion de tareas */
	xTaskCreate(Task1, (char *) "",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);
	xTaskCreate(Task2, (char *) "",
	    		configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
	    		(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

