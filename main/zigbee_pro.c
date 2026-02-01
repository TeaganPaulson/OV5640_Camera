#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
// Include necessary headers for your ESP32 and LoRa functionality 
#include "driver/uart.h" 
#include "driver/gpio.h" 
#include "ZIGBEE_PRO.h" 

#define TAG         ("ZIGBEE_PRO") 
#define UART_NUM    (   UART_NUM_2 ) 
#define buffer_size (         1024 ) 
#define TIMEOUT     (          100 ) 
static uint8_t data[buffer_size+1]  = ""; 
static int data_read                = 0; 
volatile uint32_t send_count                 = 0; 


// Function definitions 
static char num_to_str(char num) 

{ 

    switch (num) 
    { 
        case 0:  
            return '0'; 
        case 1:  
            return '1'; 
        case 2:  
            return '2'; 
        case 3:  
            return '3'; 
        case 4:  
            return '4'; 
        case 5:  
            return '5'; 
        case 6:  
            return '6'; 
        case 7:  
            return '7'; 
        case 8:  
            return '8'; 
        case 9:  
            return '9'; 
        default:  
            break; 
    } 

    return '0'; 
} 

void zigbee_init() 

{ 
    // Setup UART buffered IO with event queue 
    const int uart_buffer_size = (1024 * 2); 
    QueueHandle_t uart_queue; 

    uart_config_t uart_config = { 
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS, 
        .parity    = UART_PARITY_DISABLE, 
        .stop_bits = UART_STOP_BITS_1, 
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE 
    }; 

    gpio_config_t io_conf = { 
        .pin_bit_mask = (1ULL << GPIO_NUM_17), 
        .mode = GPIO_MODE_INPUT, 
        .pull_up_en = GPIO_PULLUP_DISABLE, 
        .pull_down_en = GPIO_PULLDOWN_ENABLE, 
        .intr_type = GPIO_INTR_DISABLE 
    }; 

    gpio_config(&io_conf); 
    // Install UART driver using an event queue here 
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0)); 
    // Configure UART parameters 
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config)); 
    // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19) 
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, GPIO_NUM_18, GPIO_NUM_17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)); 

} 

 
 

void zigbee_receive_check() 

{ 
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t*)&data_read)); 

    if(data_read>0) 
    {
        // printf("\nHere\n"); 
        if ( send_count == 0 ) 
        { 

            send_count = 1; 

        } 
        // printf("ZIGBEE Data Available: %d bytes\n", data_read); 
        // Read data from UART. 
        data_read = uart_read_bytes(UART_NUM, data, data_read, 100*TIMEOUT); 
        printf("ZIGBEE Received %d bytes: '%.*s'\n", (uint16_t)data_read, (uint16_t)data_read, (char*)data); 
    } 
    else 
    { 
        // No data received 
    } 

} 

 
 

void zigbee_send(const char* data, size_t length) 
{ 
    uart_write_bytes(UART_NUM, data, length); 
} 

 
 

void zigbee_receive(char** buffer) 
{ 

} 

 
 

void zigbee_flush() 
{ 
    ESP_ERROR_CHECK(uart_flush_input(UART_NUM)); 
} 