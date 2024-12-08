#ifndef GUI_TASK_H
#define GUI_TASK_H

#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

void gui_task_init(void);

#endif // GUI_TASK_H