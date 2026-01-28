#include <stdio.h> 
#include <string.h> 
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h" 
#include "esp_log.h" 
#include "esp_camera.h" 
#include "mbedtls/base64.h"
#include "driver/gpio.h" 
#include "sdkconfig.h" 
#include "esp_intr_types.h" 
#include "esp_intr_alloc.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_psram.h"


  // ==== CAMERA PIN CONFIGURATION FOR S3-N16R8 (Matching Adafruit Labels) ==== 
#define PWDN_GPIO_NUM         10//37
#define RESET_GPIO_NUM        9
//19
#define XCLK_GPIO_NUM         -1//42// Adjust based on your hardware
#define SIOD_GPIO_NUM         12// Adjust based on your hardware sda
#define SIOC_GPIO_NUM         11 // Adjust based on your hardware

// Camera data pins (D0–D7 → Y0–Y7)
#define Y0_GPIO_NUM            38 // D2
#define Y1_GPIO_NUM            20 // D3
#define Y2_GPIO_NUM            39 // D4
#define Y3_GPIO_NUM            21 // D5
#define Y4_GPIO_NUM            40 // D6
#define Y5_GPIO_NUM            47 // D7
#define Y6_GPIO_NUM            41 // D8
#define Y7_GPIO_NUM            4 // D9

// Synchronization pins
#define VSYNC_GPIO_NUM         1 // Frame valid
#define HREF_GPIO_NUM          2  // Line valid
#define PCLK_GPIO_NUM          45 // Pixel clock

#define TAG ("camera_example")
void app_main() {
    // Give the camera sensor time to power up
    
    // Camera configuration
    camera_config_t config = {
    .ledc_channel = -1, // Use channel 0
    .ledc_timer = -1,
    .pin_d0 = Y0_GPIO_NUM,
    .pin_d1 = Y1_GPIO_NUM,
    .pin_d2 = Y2_GPIO_NUM,  
    .pin_d3 = Y3_GPIO_NUM,
    .pin_d4 = Y4_GPIO_NUM,
    .pin_d5 = Y5_GPIO_NUM,
    .pin_d6 = Y6_GPIO_NUM,
    .pin_d7 = Y7_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,

    .xclk_freq_hz = 24000, // 24 MHz
    .pixel_format = PIXFORMAT_JPEG, // <- Change from JPEG
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 32, // Quality from 0-63 (lower means better)
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .sccb_i2c_port = 1
};

    // ESP_LOGI(TAG, "Initializing camera with config...");
    esp_err_t ret = esp_psram_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PSRAM initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        esp_camera_deinit();
        return;
    }
    
    ESP_LOGI(TAG, "Camera initialized successfully!");

    // // Configure camera for night vision
    // sensor_t *s = esp_camera_sensor_get();
    // if (s != NULL) {
    //     // Moderate settings for night vision
    //     s->set_brightness(s, 1);      // Brightness +1 (mild boost)
    //     s->set_contrast(s, 1);        // Contrast +1
    //     s->set_saturation(s, 0);      // Normal saturation
    //     s->set_special_effect(s, 0);  // No special effects
    //     s->set_whitebal(s, 1);        // Auto white balance
    //     s->set_awb_gain(s, 1);        // Auto white balance gain
    //     s->set_exposure_ctrl(s, 1);   // Auto exposure
    //     s->set_aec2(s, 0);            // Disable AEC2
    //     s->set_ae_level(s, 1);        // Auto exposure level +1
    //     s->set_agc_gain(s, 2);        // Auto gain at medium-high
    //     s->set_gainceiling(s, GAINCEILING_16X); // Moderate gain (was 32X)
    //     ESP_LOGI(TAG, "Night vision mode enabled (moderate)!");
    // }

    while (1) {
        printf("Hello, Camera!\n");
        // Capture a picture
        ESP_LOGI(TAG, "Taking a picture...");
        camera_fb_t *pic = esp_camera_fb_get();
        if (!pic) {
            ESP_LOGE(TAG, "Failed to capture picture");
            continue;
        }

        ESP_LOGI(TAG, "Picture taken! Size: %zu bytes", pic->len);

        // Print the picture data as base64 (optional)
        size_t output_len;
        unsigned char *output_buf = NULL;
        mbedtls_base64_encode(NULL, 0, &output_len, pic->buf, pic->len);
        output_buf = malloc(output_len);
        if (output_buf) {
            mbedtls_base64_encode(output_buf, output_len, &output_len, pic->buf, pic->len);
            ESP_LOGI(TAG, "Picture (Base64): %s", output_buf);
            free(output_buf);
        }

        // Return the frame buffer back to the driver
        esp_camera_fb_return(pic);

        // Wait for 10 seconds
        vTaskDelay(pdMS_TO_TICKS(100000));
    }

    // Deinitialize the camera (this will never be reached in the current loop)
    esp_camera_deinit();
}
