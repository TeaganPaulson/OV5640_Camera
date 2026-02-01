#ifndef ZIGBEE_PRO_H 
#define ZIGBEE_PRO_H 
// Includes 
#include <stdint.h> 

extern volatile uint32_t send_count;  

// Function Prototypes 
void zigbee_init(); 
void zigbee_send(const char* data, size_t length); 
void zigbee_receive_check(); 
void zigbee_receive(char** buffer); 
void zigbee_flush(); 

#endif // ZIGBEE_PRO_H 