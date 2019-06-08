/* Copyright 2017-2018, Eric Pernia
 * All rights reserved.
 *
 * This file is part of sAPI Library.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*==================[inlcusiones]============================================*/

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

// sAPI header
#include "sapi.h"
#include "Task.h"
#include "semphr.h"
#include "sapi_uart.h"

//#define SAPI_USE_INTERRUPTS


DEBUG_PRINT_ENABLE;

int main(void)
{
	// ---------- CONFIGURACIONES ------------------------------
	// Inicializar y configurar la plataforma
	boardConfig();
	// UART for debug messages
	debugPrintConfigUart( UART_USB, 115200 );
	debugPrintlnString( "Blinky con freeRTOS y sAPI." );



	uartConfig(UART_USB, 115200);
	// Cambios RO 23.5 - Seteo un callback al evento de recepcion y habilito su interrupcion
	uartRxInterruptCallbackSet(UART_USB, CallbackRx);
	// Cambios RO 23.5 todas las interrupciones de UART_USB
	uartRxInterruptSet(UART_USB, true);

	SemTxUart = xSemaphoreCreateBinary();
	SemMutexUart =xSemaphoreCreateMutex() ;
	xPointerQueue = xQueueCreate(1 , sizeof(char *));
	xTaskCreate(TaskTxUart, (const char *)"TaskTxUart",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(myTask_1, (const char *)"myTask_1",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, &xTaskHandle_RxNotify);
//	xTaskCreate(Task_ToMayusculas_OP0, (const char *)"Task_ToMayusculas_OP0",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
//	xTaskCreate(Task_ToMinusculas_OP1, (const char *)"Task_ToMinusculas_OP1",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
//	xTaskCreate(Task_ReportStack_OP2, (const char *)"Task_ToMayusculas_OP0",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
//	xTaskCreate(Task_ReportHeap_OP3, (const char *)"Task_ToMinusculas_OP1",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);

	ModuleDinamicMemory_initialize(&ModuleData,50,xQueueGenericSend,xQueueGenericSendFromISR, xQueueReceive,xQueueGenericCreate,pvPortMalloc, vPortFree);

	// Iniciar scheduler
	vTaskStartScheduler();

	while( TRUE ) {
	}

	return 0;
}
