#ifndef NOTIFY_H
#define NOTIFY_H

#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Function to start the Bluetooth notification task
void start_bluetooth_notify_task(void);

// Bluetooth notification task function prototype
void bluetooth_notify_task(void *param);

#endif // NOTIFY_H
