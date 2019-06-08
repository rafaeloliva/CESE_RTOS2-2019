
#include "DriverDinamicMemoryRTOS.h"

#include "Task1.h"



/*
 *  Pool Data - Added 4.6.2019
 */

// RAM EDU-CIAA 136KB
#define BIG_MEM_POOL_SPACE       1024
#define MED_MEM_POOL_SPACE       128
#define SMALL_MEM_POOL_SPACE     64

#define BIG_MEM_POOL_BLOCK_QTY   100
#define MED_MEM_POOL_BLOCK_QTY   50
#define SMALL_MEM_POOL_BLOCK_QTY 10


// Tamaño mayor
static uint8_t mem_for_big_pool[BIG_MEM_POOL_SPACE * SMALL_MEM_POOL_BLOCK_QTY]; // Big pool storage (Low qty blocks, of more space)
QMPool mem_pool_big;														    // Big pool ctrl-struct

// Tamaño medio
static uint8_t mem_for_med_pool[MED_MEM_POOL_SPACE * MED_MEM_POOL_BLOCK_QTY];   // Medium pool storage (med qty blocks, of med space)
QMPool mem_pool_med;														    // Medium pool ctrl-struct

// Tamaño chico
static uint8_t mem_for_small_pool[SMALL_MEM_POOL_SPACE * BIG_MEM_POOL_BLOCK_QTY];  // Small pool storage (large qty blocks, of small space)
QMPool mem_pool_small;														       // Small pool ctrl-struct


void QMPools_inicializar(void){
	//Inicialización del Pool grande
	QMPool_init(&mem_pool_big,
			mem_for_big_pool,
			sizeof(mem_for_big_pool),
			SMALL_MEM_POOL_BLOCK_QTY);
	// Otros
}
//==================================================================

void ModuleDinamicMemory_initialize( Module_Data_t *obj , uint32_t MaxLength, xQueueSendFCN xQueueSendFCN,
		xQueueSendFromISRFCN xQueueSendFromISRFCN, xQueueReceiveFCN xQueueReceiveFCN, xQueueCreateFCN xQueueCreateFCN,
		 MemoryAllocFCN MemoryAllocFCN,MemoryFreeFCN MemoryFreeFCN, Add_IncommingFrameFCN Add_IncommingFrameFCN){

	obj->xMaxStringLength = MaxLength;
	obj->xQueueCreateFunction =xQueueCreateFCN;
	obj->xQueueSendFunction = xQueueSendFCN;
	obj->xQueueSendFromISRFunction = xQueueSendFromISRFCN;
	obj->xQueueReceiveFunction = xQueueReceiveFCN;
	obj->MemoryAllocFunction = MemoryAllocFCN;
	obj->MemoryFreeFunction = MemoryFreeFCN;
	obj->Add_IncommingFrameFunction = Add_IncommingFrameFCN;
}

void ModuleDinamicMemory_send( Module_Data_t *obj ,uint8_t Isr, long * const xHigherPriorityTaskWoken, char* pbuf ,char * XpointerQueue, uint32_t portMaxDelay){
	static char* PcStringToSend = NULL;
	//if (PcStringToSend == NULL) PcStringToSend = obj->pvPortMallocFunction( obj->xMaxStringLength );
	if(PcStringToSend != NULL) strcpy(PcStringToSend ,pbuf);

	/*Si uso el enviar en una isr*/
	if(Isr) obj->xQueueSendFromISRFunction(XpointerQueue ,&PcStringToSend,xHigherPriorityTaskWoken, 0);
	else  obj->xQueueSendFunction(XpointerQueue ,&PcStringToSend,portMaxDelay, 0);
}

void ModuleDinamicMemory_send2( Module_Data_t *obj ,char *PcStringToSend, uint8_t Isr, long * const xHigherPriorityTaskWoken, char* pbuf ,char * XpointerQueue, uint32_t portMaxDelay){
	if(PcStringToSend != NULL) strcpy(PcStringToSend ,pbuf);

	/*Si uso el enviar en una isr*/
	if(Isr) obj->xQueueSendFromISRFunction(XpointerQueue ,&PcStringToSend,xHigherPriorityTaskWoken, 0);
	else  obj->xQueueSendFunction(XpointerQueue ,&PcStringToSend,portMaxDelay, 0);
}

char* ModuleDinamicMemory_receive(Module_Data_t *obj ,char * XpointerQueue, uint32_t portMaxDelay){

	char* pbuffer; /*Dato recibido*/
	obj->xQueueReceiveFunction(XpointerQueue , &pbuffer, portMaxDelay );
	return pbuffer;
}

void ModuleDinamicMemory_Free(Module_Data_t *obj , char *ultimo_mensaje){
	//obj->vPortFreeFunction(ultimo_mensaje);
}


/*
 * Wrapper #1 selects which memory allocation to use, based on QmpoolOrMalloc
 */
void* Memory_get (QMPool *const me,	uint_fast16_t const margin,	uint8_t interrupcion,uint64_t tamaniomalloc,uint8_t  QmpoolOrMalloc)
{
	QMPool  *pool;  // arreglado 04.06.2019
	char * poolmalloc;
	if(QmpoolOrMalloc){
		pool = QMPool_get(me,margin, interrupcion);

	}else {
		poolmalloc = pvPortMalloc(tamaniomalloc);

	}
	return QmpoolOrMalloc ? pool : poolmalloc;
	//return pvPortMalloc(tamaniomalloc);
}

/*
 * Wrapper #2 selects which memory_free to use, based on QmpoolOrMalloc
 */
void Memory_free (QMPool *const me,void *b,uint8_t interrupcion,void *ptrfreemalloc,uint8_t QmpoolOrMalloc){

	if(QmpoolOrMalloc){
		QMPool_put (me,b, interrupcion);

	}else {
		vPortFree(ptrfreemalloc);

	}
	//vPortFree(ptrfreemalloc);
}
