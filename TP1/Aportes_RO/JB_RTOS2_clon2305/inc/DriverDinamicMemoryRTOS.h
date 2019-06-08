
#ifndef RTOS_2_7MA_EJ_JULIAN_INC_MODULO_H_
#define RTOS_2_7MA_EJ_JULIAN_INC_MODULO_H_
#include <stdint.h>

/**/
//xQueueSend( xQueue, pvItemToQueue, xTicksToWait ) xQueueGenericSend( ( xQueue ), ( pvItemToQueue ), ( xTicksToWait ), queueSEND_TO_BACK )
typedef long (*xQueueSendFCN)( void *, const void * const , uint32_t , const long  );

//BaseType_t xQueueReceive( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait )
typedef long (*xQueueReceiveFCN)( void *, void * const , uint32_t);

//void vPortFree( void *pv )
typedef void (*vPortFreeFCN)(void *);

//void *pvPortMalloc( size_t xWantedSize )
typedef void* (*pvPortMallocFCN)(uint64_t);

//xQueueCreate( uxQueueLength, uxItemSize ) xQueueGenericCreate( ( uxQueueLength ), ( uxItemSize ), ( queueQUEUE_TYPE_BASE ) )
//	QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )

typedef void* (*xQueueCreateFCN)(const unsigned long ,const unsigned long, const uint8_t );

//BaseType_t xQueueGenericSendFromISR( QueueHandle_t xQueue, const void * const pvItemToQueue, BaseType_t * const pxHigherPriorityTaskWoken, const BaseType_t xCopyPosition )
typedef long (*xQueueSendFromISRFCN)( void *, const void * const , long * const , const long );

typedef struct {
    char *xPointerQueue;
    uint8_t xMaxStringLength;
    xQueueSendFCN xQueueSendFunction;
    xQueueSendFromISRFCN xQueueSendFromISRFunction;
    xQueueReceiveFCN xQueueReceiveFunction;
    pvPortMallocFCN pvPortMallocFunction;
    vPortFreeFCN vPortFreeFunction;
    xQueueCreateFCN xQueueCreateFunction;
}Module_Data_t;


/*Prototypes*/
void ModuleDinamicMemory_initialize( Module_Data_t *obj , uint32_t MaxLength, xQueueSendFCN xQueueSendFCN,xQueueSendFromISRFCN xQueueSendFromISRFCN, xQueueReceiveFCN xQueueReceiveFCN, xQueueCreateFCN xQueueCreateFCN, pvPortMallocFCN pvPortMallocFCN,vPortFreeFCN vPortFreeFCN);

void ModuleDinamicMemory_send( Module_Data_t *obj ,uint8_t Isr,long * const xHigherPriorityTaskWoken, char* pbuf , uint32_t portMaxDelay);

char* ModuleDinamicMemory_receive(Module_Data_t *obj, uint32_t portMaxDelay);

void ModuleDinamicMemory_Free(Module_Data_t *obj , char *ultimo_mensaje);

#endif /* RTOS_2_7MA_EJ_JULIAN_INC_MODULO_H_ */
