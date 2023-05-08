#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac.h"
#include "driver/dac_common.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define LED_PIN 2

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK GPIO_NUM_18
#define PIN_NUM_CS GPIO_NUM_5

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
    esp_err_t ret;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    printf("Initializing SD card");
    fflush(stdout);

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);

    if (ret != ESP_OK) {
        gpio_set_level(LED_PIN, 1);
        printf("Failed to initialize bus.");
        fflush(stdout);
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE("FILE", "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE("FILE", "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI("FILE", "Filesystem mounted");

    uint8_t level = 0;
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    while (1)
    {   
        gpio_set_level(2, level);
        level ^= 1;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }   

    const char *file_hello = "/sdcard/test.txt";

    sdmmc_card_print_info(stdout, card);
    fflush(stdout);

    // ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        // ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    // ESP_LOGI(TAG, "File written");

    // xTaskCreate(blink_led, "blink led", 4096, NULL, 10, &blink_handle);
    // uint8_t led = 0;
    while (1)
    {   
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }   
}
