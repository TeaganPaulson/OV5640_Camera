/**
 * @file gpio_manager.c
 * @brief GPIO management implementation for the OV5640 Camera project.
 * This file contains the implementation of GPIO configuration and management for the ESP32-CAM module, including PIR sensor handling and servo control.
 */
#include "gpio_manager.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include <time.h>
#include <sys/time.h>
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "GPIO_MANAGER";
static volatile int pir_1_flag = 0;
static volatile int pir_2_flag = 0;
static volatile int pir_3_flag = 0;

void gpio_init(void)
{
    gpio_config_t pir_conf = {
        .pin_bit_mask = (1ULL << PIR_1) | (1ULL << PIR_2) | (1ULL << PIR_3),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&pir_conf);
    
    gpio_config_t servo_conf = {
        .pin_bit_mask = (1ULL << SERVO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&servo_conf);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_1),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&led_conf);
    // Set up interrupts for PIR sensors
    gpio_set_intr_type(PIR_1, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(PIR_2, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(PIR_3, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIR_1, gpio_pir_1_isr, NULL);
    gpio_isr_handler_add(PIR_2, gpio_pir_2_isr, NULL);
    gpio_isr_handler_add(PIR_3, gpio_pir_3_isr, NULL);

}

void gpio_pir_1_isr(void* arg)
{
    pir_1_flag = 1;
}

void gpio_pir_2_isr(void* arg)
{
    pir_2_flag = 1;
}

void gpio_pir_3_isr(void* arg)
{
    pir_3_flag = 1;
}

void set_led_state(int state)
{
    gpio_set_level(LED_1, state);
}

void gpio_task(void* arg)
{
    while(1)
    {
        if (pir_1_flag) 
        {
            ESP_LOGI(TAG, "PIR 1 detected motion!");
            pir_1_flag = 0;
        }

        if (pir_2_flag) 
        {
            ESP_LOGI(TAG, "PIR 2 detected motion!");
            pir_2_flag = 0;
        }

        if (pir_3_flag) 
        {
            ESP_LOGI(TAG, "PIR 3 detected motion!");
            pir_3_flag = 0;
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void sleep_wakeup_init(void)
{
    // Configure wakeup source (e.g., timer, GPIO, etc.)
    esp_sleep_enable_ext1_wakeup_io(1ULL << PIR_1 | 1ULL << PIR_2 | 1ULL << PIR_3, ESP_EXT1_WAKEUP_ANY_HIGH); // Wake up on PIR sensor triggers
    rtc_gpio_pulldown_en(PIR_1);
    rtc_gpio_pulldown_en(PIR_2);
    rtc_gpio_pulldown_en(PIR_3);
    // Additional wakeup sources can be configured here
}