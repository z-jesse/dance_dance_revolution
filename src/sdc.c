#include "sdc.h"

#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/i2s.h"

#define PIN_NUM_MISO    GPIO_NUM_19
#define PIN_NUM_MOSI    GPIO_NUM_23
#define PIN_NUM_CLK     GPIO_NUM_18
#define PIN_NUM_CS      GPIO_NUM_5

static const char *TAG = "sdc.c";

#define MOUNT_POINT "/sdcard"

// uint8_t buffer[2][512];
// uint8_t buffer[512];
// bool buffer_full[2] = {false, false};
uint8_t write_buffer = 0;
uint8_t read_buffer = 0;
int read_index = 0;

QueueHandle_t i2s_event_queue;


// uint8_t fifo_read() {
//     if (buffer_full[read_buffer]) {
//         uint8_t data = buffer[read_buffer][read_index++];
//         if (read_index >= 512) {
//             read_buffer ^= 1;
//             read_index = 0;
//         }
//         return data;
//     }
//     ESP_LOGI(TAG, "Missed fifo_read.");
//     return 0;
// }

sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

esp_err_t sdc_init() {
    ESP_LOGI(TAG, "Initializing SD card.");

    esp_err_t ret;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

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
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ESP_FAIL;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    ESP_LOGI("FILE", "Filesystem mounted");

    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

void sdc_close() {
    // Unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    card = NULL;
    ESP_LOGI(TAG, "Card unmounted");

    spi_bus_free(host.slot);
}

void task_read_song() {
    char **songs = malloc(10 * sizeof(char *));

    DIR *dp;
    struct dirent *ep;
    dp = opendir("/sdcard");

    if (dp != NULL)
    {
        int counter = 0;
        while ((ep = readdir (dp)) != NULL) {
            songs[counter] = malloc(sizeof(strlen(ep->d_name) + 1));
            strcpy(songs[counter], ep->d_name);
            counter++;
        }
            
        songs[counter] = NULL;
            
        (void) closedir (dp);
        // return 0;
    }

    int index = 0;
    while (songs[index] != NULL)
    {
        ESP_LOGI(TAG, "index at %d = %s\n", index, songs[index]);
        index++;
    }
    
    char path[256] = "/sdcard/tpg8b8k.wav";
    // strcat(path, songs[1]);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for reading", path);
        return;
    }

    struct HEADER header;
    fread(&header, sizeof(header), 1, fp);
    printf("%s\n", header.riff);
    printf("%d\n", header.fmt_type);
    printf("%ld\n", header.sample_rate);
    printf("%d\n", header.bits_per_sample);
    fflush(stdout);

    // i2s_event_queue = xQueueCreate(10, sizeof(i2s_event_t));
    task_i2s_output(header.sample_rate);

    unsigned long count = 0;
    uint8_t buffer[512];
    bool buffer_full = false;
    while (true)
    {
        i2s_event_t event;
        if (!buffer_full) {
            size_t bytes_read = fread(buffer, 1, sizeof(buffer), fp);
            buffer_full = true;
            if (bytes_read < 512) break;
        }

        if (xQueueReceive(i2s_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            if (event.type == I2S_EVENT_TX_DONE) {
                size_t bytesOut;
                i2s_write(I2S_NUM_0, buffer, 512, &bytesOut, portMAX_DELAY);
                buffer_full = false;
                // ESP_LOGI(TAG, "read %d bytes, wrote %d bytes.\n", bytes_read, bytesOut);
                count += 512;
            }
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "%ld bytes out of %ld read.\n", count, header.size);
    fclose(fp);
    vTaskDelete(NULL);
}

void task_i2s_output(uint32_t sample_rate) {
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate = sample_rate,
        .bits_per_sample = 8,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = true,
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 4, &i2s_event_queue));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, NULL));
    ESP_ERROR_CHECK(i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN));
    i2s_zero_dma_buffer(I2S_NUM_0);
}
