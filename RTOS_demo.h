#ifndef __RTOS_DEMO_H
#define __RTOS_DEMO_H

/**
  ******************************************************************************
  * @file    RTOS_demo.h
  * @brief   FreeRTOS 知识点演示头文件
  * @note    总入口 RTOS_Demo_Start()，在 main() 中调用一次即可
  ******************************************************************************
  */

/* FreeRTOS 知识点演示：创建全部演示任务（必须在 vTaskStartScheduler() 之前调用） */
void RTOS_Demo_Start(void);

#endif /* __RTOS_DEMO_H */
