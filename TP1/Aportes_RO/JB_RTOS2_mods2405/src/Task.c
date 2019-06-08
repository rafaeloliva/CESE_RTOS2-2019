/*
 * Task.c
 *
 *  Created on: May 19, 2019
 *      Author: julian
 */




#include "Task.h"
#include "General.h"

/*Datos de Trama Recibida*/
volatile DataFrame_t Data;

/*Datos de trama para decodificar */
volatile Frame_parameters_t Frame_parameters = { '{',0 , {0,0}, NULL,NULL, '}' };

/*Semaforo Sincronizar UartTx*/
SemaphoreHandle_t SemTxUart;

/*Semaforo Sincronizar UartRx*/
SemaphoreHandle_t SemRxUart;

/*Semaforo Mutex proteger Uart*/
SemaphoreHandle_t SemMutexUart;

/*instanciar Driver memoria dinamica*/
Module_Data_t ModuleData;

/*Notificación para llegada de trama*/
TaskHandle_t xTaskHandle_RxNotify = NULL;

/*Puntero para crear la cola*/
QueueHandle_t xPointerQueue_OP0;

/*Puntero para crear la cola*/
QueueHandle_t xPointerQueue_OP1;

/*Puntero para crear la cola*/
QueueHandle_t xPointerQueue_3;


/*=================================================================================
 	 	 	 	 	 	 	 	 | Tarea  |
 =================================================================================*/

void TaskService( void* taskParmPtr )
{
	char *PtrSOF = NULL;
	char *PtrEOF = NULL;
	void* XPointerQueUe = NULL; /*Puntero auxiliar  a cola*/

	gpioWrite( LED1, ON );
	vTaskDelay( 1000 / portTICK_RATE_MS );
	gpioWrite( LED1, OFF );

	while(TRUE) {
		/*Notifica que llego trama Buena*/
		xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

		/*Proteger datos para hacer copia local*/
		taskENTER_CRITICAL();
		Frame_parameters.BufferAux = ModuleData.pvPortMallocFunction(sizeof(Data.Buffer));
		strcpy((char*)Frame_parameters.BufferAux,(const char*)Data.Buffer);
		taskEXIT_CRITICAL();

		/*Buscar posición del inicio de la trama*/
		PtrSOF = strchr((const char*)Frame_parameters.BufferAux, Frame_parameters._SOF);

		if( PtrSOF != NULL ){
			/** Decodificar T :  T[0] -'0' *10 + T[1] - '0'*/
			Frame_parameters.T[0] =  ( *(PtrSOF +  OFFSET_TAMANO)-'0' )*10 + (*(PtrSOF +  OFFSET_TAMANO + 1)-'0' ) ;

			/** Decodificar OP */
			Frame_parameters.Operation = *(PtrSOF +  OFFSET_OP)-'0';

			/* Cantidad de memoria a reservar*/
			ModuleData.xMaxStringLength = Frame_parameters.T[0] + NUM_ELEMENTOS_REST_FRAME;

			//printf( "T %d\r\n",Frame_parameters.T[0]); // QUITAR
		}

		/*Selecionar operaacion*/
		XPointerQueUe = SelecQueueFromOperation(Frame_parameters.Operation);

		if(XPointerQueUe != NULL){
			/*Envía el puntero al buffer con la trama a la cola*/
			ModuleDinamicMemory_send(&ModuleData,0,NULL,(char*)Frame_parameters.BufferAux,XPointerQueUe ,portMAX_DELAY);
		}
		/*Libero memoria del buffer aux*/
		ModuleData.vPortFreeFunction(Frame_parameters.BufferAux );
		gpioToggle( LEDB );

		/*sincronizar-Permite transmitir por uart lo que se recibe por la cola*/
		xSemaphoreGive(SemTxUart);
	}
}

/*=================================================================================
 	 	 	 	 	 	 	 	 | Tarea Mayusculizar |
 =================================================================================*/
void Task_ToMayusculas_OP0( void* taskParmPtr ){
	char * rx;
	while(1){

		rx = ModuleDinamicMemory_receive(&ModuleData,xPointerQueue_OP0,  portMAX_DELAY);
		PrintUartMessageMutex("Task_ToMayusculas_OP0", SemMutexUart);
		PrintUartBuffMutex( "mayus %s\r\n",rx,SemMutexUart);
		/*Libera memoria dinamica*/
		ModuleDinamicMemory_Free(&ModuleData, rx);
	}
}

/*=================================================================================
 	 	 	 	 	 	 	 	 | Tarea Minusculizar |
 =================================================================================*/
void Task_ToMinusculas_OP1( void* taskParmPtr ){
	char * rx;
	while(1){

		rx = ModuleDinamicMemory_receive(&ModuleData,xPointerQueue_OP1,  portMAX_DELAY);
		PrintUartMessageMutex("Task_ToMinusculas_OP1", SemMutexUart);
		PrintUartBuffMutex( "Minus %s\r\n",rx,SemMutexUart);
		/*Libera memoria dinamica*/
		ModuleDinamicMemory_Free(&ModuleData, rx);
	}
}

/*=================================================================================
 	 	 	 	 	 	 	 	 | Tarea Reportar stack disponible |
 =================================================================================*/
void Task_ReportStack_OP2( void* taskParmPtr ){

}

/*=================================================================================
 	 	 	 	 	 	 	 	 | Tarea Reportar heap disponible |
 =================================================================================*/
void Task_ReportHeap_OP3( void* taskParmPtr ){

}

/*=================================================================================
 	 	 	 	 	 	 	 	 | Tarea tx |
 =================================================================================*/
void TaskTxUart( void* taskParmPtr ){
	char * rx;
// ESTA TAREA AÚN NO HACE NADA SE
	while(true){
		/*sincronizar-Permite transmitir por uart lo que se recibe por la cola*/
		//	if( pdTRUE == xSemaphoreTake(SemTxUart,portMAX_DELAY) )
		{
			/*Recibe por la cola*/
			rx = ModuleDinamicMemory_receive(&ModuleData, xPointerQueue_3, portMAX_DELAY);
			PrintUartBuffMutex( "rx %s\r\n",rx,SemMutexUart);

			/*Libera memoria dinamica*/
			ModuleDinamicMemory_Free(&ModuleData, rx);
		}
	}
}

/*=================================================================================
 	 	 	 	 	 	 	 	 | Callback IT RX |
 =================================================================================*/
void CallbackRx( void *noUsado ){

	UBaseType_t uxSavedInterruptStatus;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	volatile char c = uartRxRead( UART_USB );  /*Char received*/

	Add_IncommingFrame(uxSavedInterruptStatus ,xHigherPriorityTaskWoken,c);

	if(xHigherPriorityTaskWoken) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
