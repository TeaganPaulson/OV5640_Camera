#include "zigbee_pro.h"
#include "avi.h"
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
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_psram.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <errno.h>

#define SDCARD_CLK              GPIO_NUM_8
#define SDCARD_CMD              GPIO_NUM_15
#define SDCARD_D0               GPIO_NUM_16
#define SDCARD_D1               GPIO_NUM_6
#define SDCARD_D2               GPIO_NUM_5
#define SDCARD_D3               GPIO_NUM_7
    
// ==== CAMERA PIN CONFIGURATION FOR S3-N16R8 (Matching Adafruit Labels) ==== 
#define PWDN_GPIO_NUM         -1//37
#define RESET_GPIO_NUM        -1
//19
#define XCLK_GPIO_NUM         0 // set the XCLK pin to the GPIO you actually wired the camera to
//                                   // the previous value was -1 which disables the clock generator.
//                                   // Without a valid XCLK the sensor will not output reliable
//                                   // JPEG data, which results in "NO-EOI" and timeouts.
#define SIOD_GPIO_NUM         14// Adjust based on your hardware sda
#define SIOC_GPIO_NUM         12
// Adjust based on your hardware

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

#define MOUNT_POINT "/sdcard"
#define TAG ("camera_example")


volatile avi_file_stream_t *stream;
static char filename[256];
static int image_number = 0;
esp_timer_handle_t timer;

void init_sdmmc_fs()
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config =
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = 40000; // 20 MHz
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk = SDCARD_CLK;
    slot_config.cmd = SDCARD_CMD;
    slot_config.d0 = SDCARD_D0;
    slot_config.d1 = SDCARD_D1;
    slot_config.d2 = SDCARD_D2;
    slot_config.d3 = SDCARD_D3;


    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if ( ret != ESP_OK )
    {
        if ( ret == ESP_FAIL )
        {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize card (%s).", esp_err_to_name(ret));
        }
    }
    else
    {
        ESP_LOGI(TAG, "SDMMC card mounted at %s", mount_point);
        sdmmc_card_print_info(stdout, card);
    }
}

int camera_setup()
{
    camera_config_t config = {
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
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
        .pin_sccb_sda = SIOD_GPIO_NUM,
        .pin_sccb_scl = SIOC_GPIO_NUM,
        .pin_pwdn = -1,
        .pin_reset = RESET_GPIO_NUM,

        .xclk_freq_hz = 24000000, // 24 MHz is what the OV5640 datasheet recommends
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_HD, // Reduced to HVGA (480x320) for reliability
        .jpeg_quality = 10,          // Moderate compression for stable operation
        .fb_count = 4,               // single frame buffer for stability
        .fb_location = CAMERA_FB_IN_PSRAM, // allocate the frame buffer in PSRAM
        .grab_mode = CAMERA_GRAB_LATEST, // wait for buffer to be empty before capturing
    };
    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        esp_camera_deinit();
        return 1;
    }
    
    ESP_LOGI(TAG, "Camera initialized successfully!");

    // // Configure camera for night vision
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); // Flip vertically
    s->set_hmirror(s, 1); // Mirror horizontally
    return 0;
}

int file_exists(const char * filename)
{
    FILE *file = fopen(filename, "r");
    if (file) 
    {
        fclose(file);
        return 1; // File exists
    }
    return 0; // File does not exist
}

void take_picture()
{
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) 
    {    
        // ESP_LOGE(TAG, "Camera capture failed");
        return;
    }

    if (stream == NULL) 
    {
        snprintf(filename, sizeof(filename), "%s/video.avi", MOUNT_POINT);

        while (file_exists(filename)) {
            image_number++;
            snprintf(filename, sizeof(filename), "%s/video_%d.avi", MOUNT_POINT, image_number);
        }

        // filename is already the full path
        stream = avi_file_stream_new(filename, "1280x720", 4);

        if (stream == NULL) 
        {
            // ESP_LOGE(TAG, "Failed to create AVI stream");
            esp_camera_fb_return(pic);
            return;
        }

        // ESP_LOGI(TAG, "Created AVI stream: %s", filename);
    }

    avi_file_stream_write_jpg_data(stream, (const char *)(pic->buf), pic->len);
    esp_camera_fb_return(pic);

    if (stream->nbr_jpgs >= 100) 
    {
        avi_file_stream_finalize(stream);
        avi_file_stream_free(stream);
        stream = NULL;
        // ESP_LOGI(TAG, "Finalized AVI stream: %s", filename);
    }
}

void app_main() 
{

    if (camera_setup() != 0) 
    {
        ESP_LOGE(TAG, "Failed to initialize camera");
        esp_restart();
        return;
    }
    init_sdmmc_fs();
    // Test SD card write
    FILE *test = fopen("/sdcard/test.txt", "w");
    if (test) {
        fprintf(test, "test");
        fclose(test);
        ESP_LOGI(TAG, "SD write OK");
    } else {
        ESP_LOGE(TAG, "SD write failed, errno: %d", errno);
    }

    const esp_timer_create_args_t timer_args = {
        .callback = (esp_timer_cb_t)take_picture,
        .name = "picture_timer"
    };

    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 250000); // 250 ms = 4 fps

    while (1) {
        
        // take_picture();

        // Wait for 10 seconds
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    esp_camera_deinit();
}
