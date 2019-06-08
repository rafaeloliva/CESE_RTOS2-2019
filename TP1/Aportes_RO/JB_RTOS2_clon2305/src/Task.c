/*
 * Task.c
 *
 *  Created on: May 19, 2019
 *      Author: julian
 */




#include "Task.h"
#include <string.h>
#include <stdlib.h>
/*Datos de Trama Recibida*/
volatile DataFrame_t Data;

/*Datos de trama para decodificar */
volatile Frame_parameters_t Frame_parameters = { '{',0 , {0,0}, NULL,NULL, '}' };

SemaphoreHandle_t SemTxUart;
SemaphoreHandle_t SemRxUart;
SemaphoreHandle_t SemMutexUart;
Module_Data_t ModuleData;
TaskHandle_t xTaskHandle_RxNotify = NULL;

QueueHandle_t xPointerQueue;
/*=================================================================================*/

void myTask_1( void* taskParmPtr )
{
	char *PtrSOF = NULL;
	char *PtrEOF = NULL;

	gpioWrite( LED1, ON );
	vTaskDelay( 1000 / portTICK_RATE_MS );
	gpioWrite( LED1, OFF );

	// Tarea periodica cada 500 ms
	portTickType xPeriodicity =  500 / portTICK_RATE_MS;
	portTickType xLastWakeTime = xTaskGetTickCount();

	while(TRUE) {
		/*Notifica que llego trama lista*/
		xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

		/*Proteger datos para hacer copia local*/
		taskENTER_CRITICAL();
		Frame_parameters.BufferAux = ModuleData.pvPortMallocFunction(sizeof(Data.Buffer));
		strcpy((char*)Frame_parameters.BufferAux,(const char*)Data.Buffer);
		taskEXIT_CRITICAL();

		/*Busco posición del inicio de la trama*/
		PtrSOF = strchr((const char*)Frame_parameters.BufferAux, Frame_parameters._SOF);

		if( PtrSOF != NULL ){
			Frame_parameters.T[0] =  ( *(PtrSOF +  OFFSET_TAMANO)-'0' )*10 + (*(PtrSOF +  OFFSET_TAMANO + 1)-'0' ) ; /*T[0] -'0' *10 + T[1] - '0'*/
			Frame_parameters.Operation = *(PtrSOF +  OFFSET_OP)-'0';
			ModuleData.xMaxStringLength = Frame_parameters.T[0] + NUM_ELEMENTOS_REST_FRAME; /* + los demas elementos del frame*/

			printf( "T %d\r\n",Frame_parameters.T[0]); // QUITAR
		}

		/*Envía el puntero al buffer con la trama a la cola*/
		ModuleDinamicMemory_send(&ModuleData,0,NULL,(char*)Frame_parameters.BufferAux, portMAX_DELAY);

		/*Libero memoria del buffer aux*/
		ModuleData.vPortFreeFunction(Frame_parameters.BufferAux );
		gpioToggle( LEDB );

		/*sincronizar-Permite transmitir por uart lo que se recibe por la cola*/
		//xSemaphoreGive(SemTxUart);
		vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
	}
}
void Task_ToMayusculas_OP0( void* taskParmPtr ){

}

void Task_ToMinusculas_OP1( void* taskParmPtr ){

}
void Task_ReportStack_OP2( void* taskParmPtr ){

}

void Task_ReportHeap_OP3( void* taskParmPtr ){

}
/*===========Task receive==================================================================================*/
void TaskTxUart( void* taskParmPtr ){
	char * rx;

	while(true){
		/*sincronizar-Permite transmitir por uart lo que se recibe por la cola*/
		//if( pdTRUE == xSemaphoreTake(SemTxUart,portMAX_DELAY) )
		{
			/*Recibe por la cola*/
			rx = ModuleDinamicMemory_receive(&ModuleData, portMAX_DELAY);

			xSemaphoreTake(SemMutexUart,portMAX_DELAY);
			printf( "rx %s\r\n",rx );
			xSemaphoreGive(SemMutexUart);

			/*Libera memoria dinamica*/
			ModuleDinamicMemory_Free(&ModuleData, rx);
		}
	}
}

void CallbackRx( void *noUsado ){

	UBaseType_t uxSavedInterruptStatus;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	char c = uartRxRead( UART_USB );

	/*Verifica Inicio de trama*/
	if( Frame_parameters._SOF == c) Data.StartFrame = 1;

	if(Data.StartFrame){
		/*Porteger acceso al buffer*/
		taskENTER_CRITICAL_FROM_ISR();
		Data.Buffer[Data.Index++]= c;
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	}
	else return;

	if(Data.Index > sizeof(Data)-1) Data.Index =0;
	Data.Buffer[Data.Index] = 0;  /*char NULL pos siguiente*/
	if(Frame_parameters._EOF == c){
		Data.StartFrame = 0;
		Data.Ready = 1;
		xTaskNotifyFromISR(xTaskHandle_RxNotify,0,eNoAction,&xHigherPriorityTaskWoken);
		Data.Index =0;
		//		xSemaphoreTakeFromISR(SemMutexUart,xHigherPriorityTaskWoken);
		//		printf( "Recibimos <<%s>> por UART\r\n", Data.Buffer );
		//		xSemaphoreGiveFromISR(SemMutexUart,xHigherPriorityTaskWoken);
	}
	if(xHigherPriorityTaskWoken) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}



