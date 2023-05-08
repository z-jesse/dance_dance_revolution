#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac.h"
#include "driver/dac_common.h"
#include "sdc.h"

#define LED_PIN 2

#define MOUNT_POINT "/sdcard"

TaskHandle_t blink_handle = NULL;

void blink_led() {
    dac_i2s_disable();
    dac_output_enable(DAC_CHANNEL_1);
    

//     uint8_t sine[128] = {128,134,140,146,152,158,165,170,
// 176,182,188,193,198,203,208,213,
// 218,222,226,230,234,237,240,243,
// 245,248,250,251,253,254,254,255,
// 255,255,254,254,253,251,250,248,
// 245,243,240,237,234,230,226,222,
// 218,213,208,203,198,193,188,182,
// 176,170,165,158,152,146,140,134,
// 128,121,115,109,103,97,90,85,
// 79,73,67,62,57,52,47,42,
// 37,33,29,25,21,18,15,12,
// 10,7,5,4,2,1,1,0,
// 0,0,1,1,2,4,5,7,
// 10,12,15,18,21,25,29,33,
// 37,42,47,52,57,62,67,73,
// 79,85,90,97,103,109,115,121,};
    

    static uint8_t led = 0;
    uint8_t index = 0;
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    return;
    while (1)
    {
        gpio_set_level(LED_PIN, led);
        led ^= 0x1;
        // dac_output_voltage(DAC_CHANNEL_1, sine[index]);
        index++;
        index = index % 128;
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }   
}

void app_main() {
    sdc_init();

    const char *file_test = MOUNT_POINT"/test.txt";

    read_file(file_test);

    // xTaskCreate(blink_led, "blink led", 4096, NULL, 10, &blink_handle);
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
