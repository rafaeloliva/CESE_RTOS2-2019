/*
 * General.c
 *
 *  Created on: May 24, 2019
 *      Author: julian
 */

#include "General.h"


/*=================================================================================
						Almacena en el buffer de la RX ISR
=================================================================================*/
void Add_IncommingFrame(UBaseType_t uxSavedInterruptStatus ,BaseType_t xHigherPriorityTaskWoken, volatile char c){

	/*Verifica Inicio de trama*/
	if( Frame_parameters._SOF == c) Data.StartFrame = 1;

	if(Data.StartFrame){

		/*Proteger acceso al buffer*/
		taskENTER_CRITICAL_FROM_ISR();
		Data.Buffer[Data.Index++]= c;
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	}
	else return;

	if(Data.Index > sizeof(Data)-1) Data.Index =0;  /*Garantiza no desbordamiento del buffer*/

	Data.Buffer[Data.Index] = 0; 					/*char NULL pos siguiente*/

	if(Frame_parameters._EOF == c){
		Data.StartFrame = 0;
		Data.Ready = 1;
		/*Frame buena en el buffer*/
		xTaskNotifyFromISR(xTaskHandle_RxNotify,0,eNoAction,&xHigherPriorityTaskWoken);
		Data.Index =0;
	} /*Si el paquete no tiene elfin descartara*/

}


/*=================================================================================
 	 	 	 	 	 	selecionar puntero a cola segun operacion
 =================================================================================*/
void* SelecQueueFromOperation(Enum_Op_t OP){
	void * XpointerSelected = NULL;
	switch(OP){

	case OP0:
		XpointerSelected = xPointerQueue_OP0;
		break;
	case OP1:
		XpointerSelected = xPointerQueue_OP1;
		break;
	case OP2:
		break;
	case OP3:
		break;
	}
	return XpointerSelected;

}

/*=================================================================================
 	 	 	 	 	 	 	 	 packetToLower
 =================================================================================*/
void packetToLower(uint8_t *ptrToPacketLower){

	uint16_t tSizePacket;
	uint8_t i;
	tSizePacket=*(ptrToPacketLower+3)*256;
	tSizePacket=tSizePacket+*(ptrToPacketLower+2);
	for(i=0;i<tSizePacket;i++){
		if(*(ptrToPacketLower+i+4)>=41 &&  *(ptrToPacketLower+i+4)<=90)
			*(ptrToPacketLower+i+4)=*(ptrToPacketLower+i+4)+32;
	}

}
/*=================================================================================
 	 	 	 	 	 	 	 	 packetToUpper
 =================================================================================*/
void packetToUpper(uint8_t *ptrToPacketUpper){
	uint16_t tSizePacket;
	uint8_t i;
	tSizePacket=*(ptrToPacketUpper+3)*256;
	tSizePacket=tSizePacket+*(ptrToPacketUpper+2);
	for(i=0;i<tSizePacket;i++){
		if(*(ptrToPacketUpper+i+4)>=97 &&  *(ptrToPacketUpper+i+4)<=122)
			*(ptrToPacketUpper+i+4)=*(ptrToPacketUpper+i+4)-32;
	}
}
/*=================================================================================
 	 	 	 	 	 	 	 	 Print string buffer + message con mutex
 =================================================================================*/
void PrintUartBuffMutex(char * Message,char *Buf, SemaphoreHandle_t SemMutexUart){
	xSemaphoreTake(SemMutexUart,portMAX_DELAY);
	printf(Message,Buf );
	xSemaphoreGive(SemMutexUart);
}
/*=================================================================================
 	 	 	 	 	 	 	 	 Print only message con mutex
 =================================================================================*/
void PrintUartMessageMutex(char * Message, SemaphoreHandle_t SemMutexUart){
	xSemaphoreTake(SemMutexUart,portMAX_DELAY);
	printf(Message);
	xSemaphoreGive(SemMutexUart);
}
