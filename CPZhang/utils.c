#include "main.h"

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
