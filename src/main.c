#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac.h"
#include "driver/dac_common.h"
#include "sdc.h"
#include "driver/i2s.h"

#define LED_PIN 2

#define MOUNT_POINT "/sdcard"

TaskHandle_t blink_handle = NULL;
TaskHandle_t sdc_handle = NULL;

void blink_led() {

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate = 8000,
        .bits_per_sample = 8,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = true,
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, NULL));
    ESP_ERROR_CHECK(i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN));

    // dac_output_enable(DAC_CHANNEL_1);

    while (1)
    {
        if (buffer_avail()) {
            size_t bytesOut;
            ESP_ERROR_CHECK(i2s_write(I2S_NUM_0, get_buffer(), 512, &bytesOut, portMAX_DELAY));
        }
        // dac_output_voltage(DAC_CHANNEL_1, fifo_read());
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }   
}

void app_main() {
    // sdc_init();
    
    // const char *file_test = MOUNT_POINT"/test.txt";

    // read_file(file_test);
    xTaskCreate(sdc_init, "sdc", 4096, NULL, 10, &sdc_handle);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(blink_led, "blink led", 4096, NULL, 10, &blink_handle);
    // uint8_t led = 0;

    // End loop
    uint8_t level = 0;
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    while (1)
    {   
        gpio_set_level(2, level);
        level ^= 1;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }   
}
