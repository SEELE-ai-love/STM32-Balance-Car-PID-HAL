/**
  ******************************************************************************
  * @file    RTOS_demo.c
  * @brief   FreeRTOS 知识点演示（学习用，不参与平衡车控制逻辑）
  * @author
  * @note    每个知识点独立成任务/函数，按 1~5 编号分开，方便对照学习。
  *          默认不运行：main.c 中把 RTOS_DEMO_ENABLE 改为 1 后重新编译烧录。
  *          观察方式：USB串口(USART1, 9600) 看 printf 打印 + LED(PC13) 闪烁。
  *
  *          知识点列表：
  *            1. 任务创建与删除   xTaskCreate / vTaskDelete
  *            2. 延时阻塞         vTaskDelay(相对) / vTaskDelayUntil(绝对)
  *            3. 队列通信         xQueueCreate / xQueueSend / xQueueReceive
  *            4. 任务挂起与恢复   vTaskSuspend / vTaskResume
  *            5. 二值信号量       xSemaphoreCreateBinary / xSemaphoreGive / xSemaphoreTake
  ******************************************************************************
  */

#include "main.h"
#include "LED.h"
#include "RTOS_demo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdio.h>

/* ======================================================================
 * 知识点 1：任务创建与删除
 *   xTaskCreate(任务函数, 名字, 栈大小(字), 参数, 优先级, 任务句柄)
 *   vTaskDelete(NULL)      -> 删除当前任务自己
 *   vTaskDelete(句柄)      -> 删除指定任务
 * ====================================================================== */

/* 子任务：被"父任务"创建，运行一会儿后自我删除 */
static void Demo_Child_Task(void *argument)
{
	uint8_t i;

	(void)argument;

	for (i = 0; i < 5; i++)
	{
		printf("[1] 子任务运行中, 计数=%d\r\n", i);
		vTaskDelay(pdMS_TO_TICKS(500));		/* 先借用知识点2的延时，让打印能看见 */
	}

	/* vTaskDelete(NULL)：参数为 NULL 表示删除"当前正在运行的这个任务"自己 */
	printf("[1] 子任务运行结束, 自我删除 vTaskDelete(NULL)\r\n");
	vTaskDelete(NULL);
}

/* 父任务：循环创建子任务，并演示用句柄删除指定任务 */
static void Demo_CreateDelete_Task(void *argument)
{
	TaskHandle_t childHandle = NULL;		/* 子任务句柄：创建时由内核写回 */
	uint8_t round;

	(void)argument;

	for (round = 0; round < 3; round++)
	{
		printf("[1] 第%d轮: xTaskCreate 创建子任务\r\n", round + 1);

		/* 创建子任务：
		 *   函数名     Demo_Child_Task
		 *   任务名     "Child"
		 *   栈大小     128 字 = 512 字节
		 *   参数       NULL
		 *   优先级     2
		 *   句柄       &childHandle（可 NULL，不需要句柄就传 NULL）
		 */
		xTaskCreate(Demo_Child_Task, "Child", 128, NULL, 2, &childHandle);

		/* 等子任务自己跑完（子任务约 2.5 秒后自我删除） */
		vTaskDelay(pdMS_TO_TICKS(3000));

		if (childHandle != NULL)
		{
			/* 若子任务还没删完，父任务用句柄强制删除它 */
			printf("[1] 父任务用句柄删除子任务 vTaskDelete(childHandle)\r\n");
			vTaskDelete(childHandle);
			childHandle = NULL;
		}
	}

	/* 演示结束，父任务也删除自己（任务创建多了要及时回收） */
	printf("[1] 创建/删除演示结束, 父任务自我删除\r\n");
	vTaskDelete(NULL);
}

/* ======================================================================
 * 知识点 2：延时阻塞
 *   vTaskDelay(xTicksToDelay)          相对延时：从"本次调用"开始延时 x 个节拍
 *   vTaskDelayUntil(pxLast, xIncrement) 绝对延时：固定周期，不受任务体耗时影响
 *   延时期间任务进入"阻塞态"，不占用 CPU，这是 RTOS 与裸机 delay 的本质区别
 * ====================================================================== */

static void Demo_Delay_Task(void *argument)
{
	TickType_t xLastWakeTime;
	uint32_t count = 0;

	(void)argument;

	/* 先取当前节拍计数，作为绝对延时的基准 */
	xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		count++;
		printf("[2] 绝对延时 vTaskDelayUntil(1000), count=%lu\r\n", count);
		LED_Turn();

		/* ---- 方式一：绝对延时（固定周期，推荐用于精确控制）----
		 * 以 xLastWakeTime 为基准，等到"基准+周期"才唤醒，
		 * 任务体耗时不会累积，周期恒定。
		 * 注意：任务体总耗时不能超过周期，否则会错过唤醒点。
		 * xLastWakeTime 会被函数自动更新，无需手动赋值。 */
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));

		/* ---- 方式二：相对延时 ----
		 * vTaskDelay(x) 从"这一句执行到"开始延时 x 个节拍，
		 * 任务体耗时会被累加，实际周期 = 任务体耗时 + 1000ms，
		 * 周期会有漂移。把上面 vTaskDelayUntil 换成
		 * vTaskDelay(pdMS_TO_TICKS(1000)) 即可体验区别。 */
	}
}

/* ======================================================================
 * 知识点 3：队列（任务间通信）
 *   一个任务往里放数据(xQueueSend)，另一个任务取数据(xQueueReceive)
 *   - 队列为空：接收任务阻塞等待（不占 CPU）
 *   - 队列已满：发送任务可指定阻塞等待超时
 * ====================================================================== */

static QueueHandle_t demo_queue;		/* 队列句柄：全局，供发送/接收两个任务共用 */

/* 发送任务：每 500ms 往队列里发一个递增数字 */
static void Demo_Queue_Send_Task(void *argument)
{
	uint32_t num = 0;

	(void)argument;

	for (;;)
	{
		num++;

		/* xQueueSend(队列, 要发送的数据地址, 等待时间)
		 * 队列满时最多等 100ms，还塞不进去就返回 errQUEUE_FULL */
		if (xQueueSend(demo_queue, &num, pdMS_TO_TICKS(100)) == pdPASS)
		{
			printf("[3] 发送任务: 已发送 %lu\r\n", num);
		}
		else
		{
			printf("[3] 发送任务: 队列满, 本次发送失败\r\n");
		}

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/* 接收任务：一直阻塞在队列上，有数据就取出并打印 */
static void Demo_Queue_Receive_Task(void *argument)
{
	uint32_t recv = 0;

	(void)argument;

	for (;;)
	{
		/* xQueueReceive(队列, 接收数据存放地址, 等待时间)
		 * portMAX_DELAY = 无限期等待：队列没数据就一直阻塞 */
		if (xQueueReceive(demo_queue, &recv, portMAX_DELAY) == pdPASS)
		{
			printf("[3] 接收任务: 收到 %lu\r\n", recv);
			LED_Turn();
		}
	}
}

/* ======================================================================
 * 知识点 4：任务挂起与恢复
 *   vTaskSuspend(句柄)  挂起任务：任务进入"挂起态"，不再被调度，必须手动恢复
 *   vTaskResume(句柄)   恢复任务：从挂起态回到就绪态
 *   典型场景：暂停/继续某个功能（如暂停数据采集）
 * ====================================================================== */

static TaskHandle_t suspend_task_handle;	/* 被挂起任务的句柄 */

/* 演示任务：正常运行，会被下面的控制任务挂起/恢复 */
static void Demo_Suspend_Task(void *argument)
{
	uint32_t count = 0;

	(void)argument;

	for (;;)
	{
		count++;
		printf("[4] 演示任务运行中, count=%lu\r\n", count);
		LED_Turn();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/* 控制任务：每 5 秒挂起演示任务 3 秒，再恢复 */
static void Demo_SuspendCtrl_Task(void *argument)
{
	(void)argument;

	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(5000));

		/* 挂起指定任务：此后它不再运行，打印也会停住 */
		printf("[4] >>> vTaskSuspend() 挂起演示任务, 暂停3秒\r\n");
		vTaskSuspend(suspend_task_handle);

		vTaskDelay(pdMS_TO_TICKS(3000));

		/* 恢复指定任务：重新进入就绪态，继续从暂停处运行 */
		printf("[4] >>> vTaskResume() 恢复演示任务\r\n");
		vTaskResume(suspend_task_handle);
	}
}

/* ======================================================================
 * 知识点 5：二值信号量（任务同步）
 *   信号量只有"有(1)/无(0)"两种状态，用于任务之间"发事件、等事件"
 *   - xSemaphoreGive()      释放/给出信号量（通常由"事件源"调用）
 *   - xSemaphoreTake()      获取/等待信号量（任务阻塞等待，直到有人 Give）
 *   - 中断里请用 xSemaphoreGiveFromISR()（不能在 ISR 调用阻塞型 API）
 * ====================================================================== */

static SemaphoreHandle_t demo_sem;		/* 二值信号量句柄 */

/* 事件源任务：模拟"外部事件"，每 2 秒给出一次信号量 */
static void Demo_Sem_Event_Task(void *argument)
{
	(void)argument;

	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(2000));

		/* 给出信号量：把信号量从"无"置为"有"。
		 * 如果有任务正阻塞在 xSemaphoreTake 上，它会被立即唤醒。
		 * 若在中断中产生事件，应改用：
		 *   BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		 *   xSemaphoreGiveFromISR(demo_sem, &xHigherPriorityTaskWoken);
		 *   portYIELD_FROM_ISR(xHigherPriorityTaskWoken); */
		printf("[5] 事件源: 释放二值信号量 xSemaphoreGive()\r\n");
		xSemaphoreGive(demo_sem);
	}
}

/* 等待任务：阻塞在信号量上，拿到一次信号量就处理一次 */
static void Demo_Sem_Task(void *argument)
{
	uint32_t count = 0;

	(void)argument;

	for (;;)
	{
		/* 等待信号量：没有信号量时任务进入阻塞态（不占 CPU）
		 * 取到一次信号量后，信号量立即回到"无"状态（这就是"二值"） */
		if (xSemaphoreTake(demo_sem, portMAX_DELAY) == pdPASS)
		{
			count++;
			printf("[5] 信号量任务: 获得信号量, count=%lu\r\n", count);
			LED_Turn();
		}
	}
}

/* ======================================================================
 * 总入口：创建上面 5 个知识点对应的全部演示任务
 * 调用位置：main() 里 vTaskStartScheduler() 之前
 * ====================================================================== */
void RTOS_Demo_Start(void)
{
	/* 1. 创建/删除演示（父任务自己会创建子任务，这里只创建父任务） */
	xTaskCreate(Demo_CreateDelete_Task, "DemoCreate", 256, NULL, 1, NULL);

	/* 2. 延时阻塞演示 */
	xTaskCreate(Demo_Delay_Task, "DemoDelay", 256, NULL, 1, NULL);

	/* 3. 队列演示：先创建队列(长度4, 元素uint32_t)，再创建发送/接收两个任务 */
	demo_queue = xQueueCreate(4, sizeof(uint32_t));
	if (demo_queue != NULL)
	{
		xTaskCreate(Demo_Queue_Send_Task, "DemoQSnd", 256, NULL, 1, NULL);
		xTaskCreate(Demo_Queue_Receive_Task, "DemoQRcv", 256, NULL, 1, NULL);
	}

	/* 4. 挂起/恢复演示：先创建"被挂起"的任务拿到句柄，再创建控制任务 */
	xTaskCreate(Demo_Suspend_Task, "DemoSuspend", 256, NULL, 1, &suspend_task_handle);
	xTaskCreate(Demo_SuspendCtrl_Task, "DemoSuspendCtrl", 256, NULL, 1, NULL);

	/* 5. 二值信号量演示：先创建信号量，再创建事件源和等待任务 */
	demo_sem = xSemaphoreCreateBinary();
	if (demo_sem != NULL)
	{
		xTaskCreate(Demo_Sem_Event_Task, "DemoSemEvt", 256, NULL, 1, NULL);
		xTaskCreate(Demo_Sem_Task, "DemoSem", 256, NULL, 1, NULL);
	}
}
