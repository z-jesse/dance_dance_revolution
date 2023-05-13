#ifndef SDC_H
#define SDC_H

#include <stdbool.h>
#include "esp_check.h"

struct HEADER
{
    char riff[4];
    uint32_t size;
    char wave[4];
    char fmt_chunk_marker[4];
    uint32_t fmt_length;
    uint16_t fmt_type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byterate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_chunk_header[4];
    uint32_t data_size;
};

uint8_t fifo_read();

bool buffer_avail();

uint8_t * get_buffer();

void sdc_init();

esp_err_t read_file(const char *path);

#endif
