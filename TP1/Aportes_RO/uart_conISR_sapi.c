// esto iría en el main, con sapi.h incluido..

// Arranque en Sapi con UARTS, en main
// Inicializar y configurar la plataforma
boardConfig();

// Configuracion UART
debugPrintConfigUart( UART_USB, 115200 );

// Gestión de interrupciones
uartRxInterruptCallbackSet( UART_USB, recibir_UART );
uartRxInterruptSet( UART_USB, true );

// Envio con Transmisor vacio..
uartTxInterruptCallbackSet( UART_USB, enviar_UART );
uartTxInterruptSet( UART_USB, true );


// definir 
void recibir_UART(void){
}

void enviar_UART(void){
}