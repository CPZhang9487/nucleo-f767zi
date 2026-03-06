#include "main.h"

#include "FreeRTOS.h"  // 未使用 FreeRTOS 時請自行註解

/* 
 * 重定向 print 輸出到串口
 * 自行替換使用的串口 PRINT_HUART
 * 在啟用 FreeRTOS 為前提下，若要使用非阻塞 print，請將 PRINT_NONBLOCK 設定為 1 並設定好串口中斷
 * 須注意若啟用非阻塞 print，此檔案會使用 HAL_UART_TxCpltCallback
 */
#define PRINT_HUART huart3
#define PRINT_NONBLOCK 1

extern UART_HandleTypeDef PRINT_HUART;

#if defined(INC_FREERTOS_H) && PRINT_NONBLOCK

#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "task.h"

static QueueHandle_t hPrintQueue = NULL;
static TaskHandle_t hPrintTask = NULL;

typedef struct {
    char *ptr;
    int len;
} Print_t;

void printTask() {
    for (;;) {
        if (hPrintQueue == NULL) {
            hPrintTask = NULL;
            vTaskDelete(NULL);
        }

        Print_t *print;
        if (xQueueReceive(hPrintQueue, &print, portMAX_DELAY) == pdPASS) {
            HAL_UART_Transmit_IT(&PRINT_HUART, (uint8_t *)print->ptr, print->len);
            xTaskNotifyWait(
                0,
                0xffffffff,
                NULL,
                portMAX_DELAY
            );
            free(print->ptr);
            free(print);
        }
    }

    hPrintTask = NULL;
    vTaskDelete(NULL);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &PRINT_HUART) 
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xTaskNotifyFromISR(
            hPrintTask,
            0,
            eNoAction,
            &xHigherPriorityTaskWoken
        );

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#endif  // #if defined(INC_FREERTOS_H) && PRINT_NONBLOCK

int _write(int file, char *ptr, int len) {
    (void)file;

#if defined(INC_FREERTOS_H) && PRINT_NONBLOCK

    if (hPrintTask == NULL) {
        if (xTaskCreate(printTask, "print", 256, NULL, 0, &hPrintTask) == pdPASS) {
            hPrintQueue = xQueueCreate(100, sizeof(Print_t*));
        }
    }

    if (hPrintQueue == NULL) {
        if (hPrintTask != NULL) {
            vTaskDelete(hPrintTask);
            hPrintTask = NULL;
        }
        return 0;
    }

    Print_t *print = malloc(sizeof(Print_t));
    if (print == NULL) {
        return 0;
    }

    print->ptr = malloc(sizeof(char) * len);
    print->len = len;
    if (print->ptr == NULL) {
        free(print);
        return 0;
    }

    memcpy(print->ptr, ptr, len);
    xQueueSend(hPrintQueue, &print, 0);

#else

    HAL_UART_Transmit(&PRINT_HUART, (uint8_t *)ptr, len, HAL_MAX_DELAY);

#endif  // #if defined(INC_FREERTOS_H) && PRINT_NONBLOCK

    return len;
}

/*
 * 改寫 malloc 與 free 成 FreeRTOS 的 pvPortMalloc 與 vPortFree
 * 支援 newlib reentrant
 */
#ifdef PORTABLE_H

#if defined(configUSE_NEWLIB_REENTRANT) && configUSE_NEWLIB_REENTRANT
void *_malloc_r(struct _reent *r, size_t xSize) {
    (void)r;
#else
void *malloc(size_t xSize) {
#endif
    return pvPortMalloc(xSize);
}

#if defined(configUSE_NEWLIB_REENTRANT) && configUSE_NEWLIB_REENTRANT
void _free_r(struct _reent *r, void *pv) {
    (void)r;
#else
void free(void *pv) {
#endif
    vPortFree(pv);
}

#endif  // #ifdef PORTABLE_H
