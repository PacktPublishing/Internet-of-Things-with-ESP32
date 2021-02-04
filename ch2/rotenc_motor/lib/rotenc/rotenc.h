
#ifndef rotenc_h_
#define rotenc_h_

#include "freertos/task.h"

void rotenc_init(int clk, int dt);

void rotenc_setPosChangedTask(TaskHandle_t pos_changed_task);

int rotenc_getPos(void);

#endif