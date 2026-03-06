#include "main.h"

#include "FreeRTOS.h"  // 未使用 FreeRTOS 時請自行註解

/* 
 * 重定向 print 輸出到串口
 * 自行替換使用的串口 PRINT_HUART
 */
#define PRINT_HUART huart3

extern UART_HandleTypeDef PRINT_HUART;

int _write(int file, char *ptr, int len) {
    (void)file;

    HAL_UART_Transmit(&PRINT_HUART, (uint8_t *)ptr, len, HAL_MAX_DELAY);

    return len;
}

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

#endif
