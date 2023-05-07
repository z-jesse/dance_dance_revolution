#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define LED_PIN 2

TaskHandle_t blink_handle = NULL;

void blink_led() {
    static uint8_t led = 0;
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    while (1)
    {
        gpio_set_level(LED_PIN, led);
        led ^= 0x1;
        vTaskDelay(10);
    }   
}

void app_main() {
    xTaskCreate(blink_led, "blink led", 4096, NULL, 10, &blink_handle);
    while (1)
    {
        taskYIELD();
    }   
}
