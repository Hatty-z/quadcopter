#include "dwt_time.h"

#define SystemCoreClock 84000000

void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief 获取当前时间（秒）
 * @note 通过DWT（数据观察与跟踪）计数器获取系统运行时间
 * @return float 返回从系统启动开始经过的时间（秒）
 */
uint32_t DWT_GetTime(void)
{
    return DWT->CYCCNT;  // 将DWT计数器的值转换为秒
}
