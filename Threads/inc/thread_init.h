#ifndef __THREAD_INIT_H__
#define __THREAD_INIT_H__

#include "main.h"

#define ENABLE_INT() __set_PRIMASK(0) /* 使能全局中断 */
#define DISABLE_INT() __set_PRIMASK(1) /* 禁止全局中断 */


extern const volatile uint32_t AppAddr;

void JumpToApp(void);
void thread_init(ULONG input);



#endif

