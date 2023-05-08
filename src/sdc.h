#ifndef SDC_H
#define SDC_H

#include "esp_check.h"

void sdc_init();

esp_err_t read_file(const char *path);

#endif
