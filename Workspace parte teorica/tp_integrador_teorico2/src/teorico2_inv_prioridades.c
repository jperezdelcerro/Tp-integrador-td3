/*
 * En este ejemplo se puede observar la inversion de prioridades.
 *
 */


#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#define ADC_H_

#define ADC_CH0_PORT  0
#define ADC_CH0_PIN  23

#define ADC_CH1_PORT  0
#define ADC_CH1_PIN  24

#define ADC_CH2_PORT  0
#define ADC_CH2_PIN  25

#define ADC_CH3_PORT  0
#define ADC_CH3_PIN  26

#define ADC_CH4_PORT  1
#define ADC_CH4_PIN  30

#define ADC_CH5_PORT  1
#define ADC_CH5_PIN  31

#define ADC_CH6_PORT  0
#define ADC_CH6_PIN   3

#define ADC_CH7_PORT  0
#define ADC_CH7_PIN   2

void init_adc(void);

xQueueHandle queueADC;
xSemaphoreHandle mutex;

void init_adc(void)
{
	static ADC_CLOCK_SETUP_T ADCSetup;

	/* Asocia el pin a la función de ADC */
	Chip_IOCON_PinMux(LPC_IOCON,ADC_CH0_PORT,ADC_CH0_PIN,IOCON_MODE_INACT,IOCON_FUNC1);

	/* Inicializa el ADC */
	Chip_ADC_Init(LPC_ADC, &ADCSetup);

	/* Conecta el ADC con el canal elegido */
	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH0, ENABLE);

	/* Velocidad de conversión máxima */
	Chip_ADC_SetSampleRate(LPC_ADC, &ADCSetup, ADC_MAX_SAMPLE_RATE);

	/* Desactiva modo ráfaga */
	Chip_ADC_SetBurstCmd(LPC_ADC, DISABLE);

}


static void Task1(void *pvParameters){

	uint16_t dataADC;

	while(1){

		/* Aseguro el recurso ADC tomando el semaforo */
		xSemaphoreTake(mutex, portMAX_DELAY);

		/* Lee el valor del ADC y lo guarda en la variable dataADC */
		Chip_ADC_ReadValue(LPC_ADC, ADC_CH0, &dataADC);

		/* Escribo el valor en la cola */
		xQueueSendToBack(queueADC,&dataADC,portMAX_DELAY);

		/* Libero el semaforo una vez que termine de utilizar el ADC */
		xSemaphoreGive(mutex);
		}
}



static void Task2(void *pvParameters){

	uint16_t dataADC;

	/*Fuerzo la inversion de prioridades*/
	vTaskDelay(1000/portTICK_RATE_MS);

	while(1){

		/* Aseguro el recurso ADC tomando el semaforo */
		xSemaphoreTake(mutex, portMAX_DELAY);

		/* Lee el valor del ADC y lo guarda en la variable dataADC */
		Chip_ADC_ReadValue(LPC_ADC, ADC_CH0, &dataADC);

		/* Escribo el valor en la cola */
		xQueueSendToBack(queueADC,&dataADC,portMAX_DELAY);

		/* Libero el semaforo una vez que termine de utilizar el ADC */
		xSemaphoreGive(mutex);
		}

	}


static void Task3(void *pvParameters){

	uint16_t dataADC;

	/*Fuerzo la inversion de prioridades*/
	vTaskDelay(500/portTICK_RATE_MS);

	while(1){
		/* Recibo el valor de la cola */
		xQueueReceive(queueADC,&dataADC,portMAX_DELAY);
	}

}


/****************************************************************************************************/
/**************************************** MAIN ******************************************************/
/****************************************************************************************************/

int main(void){

	/* Levanta la frecuencia del micro */
	SystemCoreClockUpdate();

	init_adc();

	/* Creacion del semaforo mutex */
	mutex = xSemaphoreCreateMutex();

	queueADC = xQueueCreate(1,sizeof(uint16_t));
    /* Creacion de tareas */
	xTaskCreate(Task1, (char *) "",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);
	xTaskCreate(Task2, (char *) "",
	    		configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
	    		(xTaskHandle *) NULL);

	xTaskCreate(Task3, (char *) "",
	    		configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL),
	    		(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

