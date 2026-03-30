/**
 * @file gpio_manager.h
 * @brief Header file for GPIO management in the OV5640 Camera project.
 * This file contains function declarations and definitions related to GPIO configuration and management for the ESP32-CAM module.
 */
#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H

#include "driver/gpio.h" 

#define PIR_1 GPIO_NUM_9
#define PIR_2 GPIO_NUM_10
#define PIR_3 GPIO_NUM_11

#define SERVO GPIO_NUM_13
#define LED_1 GPIO_NUM_19

/**
 * @brief Initializes GPIO pins for PIR sensors, servo control, and LED.
 * This function configures the specified GPIO pins for input/output and sets up interrupts for the PIR sensors.
 * @return void
 */
void gpio_init(void);
/**
 * @brief ISR for PIR sensor 1.
 * This function is called when a rising edge is detected on the PIR_1 pin, setting the corresponding flag.
 * @param arg Unused parameter for ISR compatibility.
 * @return void
 */
void gpio_pir_1_isr(void* arg);
/**
 * @brief ISR for PIR sensor 2.
 * This function is called when a rising edge is detected on the PIR_2 pin, setting the corresponding flag.
 * @param arg Unused parameter for ISR compatibility.
 * @return void
 */
void gpio_pir_2_isr(void* arg);
/**
 * @brief ISR for PIR sensor 3.
 * This function is called when a rising edge is detected on the PIR_3 pin, setting the corresponding flag.
 * @param arg Unused parameter for ISR compatibility.
 * @return void
 */
void gpio_pir_3_isr(void* arg);
/**
 * @brief Task for handling GPIO operations.
 * This function contains the main logic for managing GPIO operations in a separate task.
 * @param arg Unused parameter for task compatibility.
 * @return void
 */
void gpio_task(void* arg);
/**
 * @brief Sets the state of the LED.
 * This function controls the state of the LED connected to LED_1 pin based on the provided state parameter.
 * @param state The desired state of the LED (0 for off, 1 for on).
 * @return void
 */
void set_led_state(int state);
/**
 * @brief Initializes sleep wakeup functionality.
 * This function configures the necessary settings for enabling sleep wakeup functionality on the ESP32-C
 */
void sleep_wakeup_init(void);

#endif // GPIO_MANAGER_H