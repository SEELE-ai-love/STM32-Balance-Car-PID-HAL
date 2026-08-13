#include "main.h"
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "Motor.h"
#include "MPU6050.h"
#include "Encoder.h"
#include "BlueSerial.h"
#include "PID.h"
#include "USB.h"
#include "HC_SR04.h"
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_demo.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>


/******************** RTOS 学习演示开关 ********************/
/* 1 = 运行 FreeRTOS 知识点演示（队列/延时/挂起/建删任务/信号量）
 * 0 = 关闭（不影响平衡车控制任务） */
#define RTOS_DEMO_ENABLE  0


/******************** 系统变量 ********************/

volatile uint32_t system_tick = 0;




/******************** MPU6050 ********************/

int16_t AX, AY, AZ;
int16_t GX, GY, GZ;


float AngleAcc;
float AngleAcc_Filter;

float AngleGyro;
float Angle;

float Alpha;



/******************** 电机 ********************/

int16_t LeftPWM;
int16_t RightPWM;

int16_t AvePWM;
int16_t DifPWM;



/******************** 编码器 ********************/

float LeftSpeed;
float RightSpeed;

float AveSpeed;
float DifSpeed;



/******************** 状态 ********************/

uint8_t KeyNum;
uint8_t RunFlag = 0;


/******************** 超声波 ********************/


float Distance;



/******************** PID ********************/


PID_t AnglePID =
{
	.Kp = 3,
	.Ki = 0.2,
	.Kd = 2.5,

	.OutMax = 100,
	.OutMin = -100,

	.OutOffset = 4,

	.ErrorIntMax = 60,
	.ErrorIntMin = -60,
};



PID_t SpeedPID =
{
	.Kp = 2,
	.Ki = 0.05,
	.Kd = 0,

	.OutMax = 20,
	.OutMin = -20,

	.ErrorIntMax = 500,
	.ErrorIntMin = -500,
};



PID_t TurnPID =
{
	.Kp = 3,
	.Ki = 2,
	.Kd = 0,

	.OutMax = 100,
	.OutMin = -100,

	.ErrorIntMax = 30,
	.ErrorIntMin = -30,
};


/******************** 距离环(超声波控距) ********************/


PID_t DistancePID =
{
	.Kp = 3,
	.Ki = 0.05,
	.Kd = 0,

	.OutMax = 8,
	.OutMin = -8,

	.ErrorIntMax = 200,
	.ErrorIntMin = -200,
};



/******************** 函数声明 ********************/


void SystemClock_Config(void);

void MPU6050_Update(void);

void Angle_Update(void);

void Angle_PID_Control(void);

void Speed_PID_Control(void);

void Distance_PID_Control(void);

void OLED_Show(void);


/******************** 系统时钟配置 ********************/
/**
  * 函    数：系统时钟配置
  * 说    明：外部8MHz晶振，PLL倍频9倍，SYSCLK=72MHz
  *          AHB=72MHz，APB1=36MHz，APB2=72MHz，Flash等待2个周期
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/* 配置外部高速晶振HSE和PLL */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/* 配置总线时钟分频 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
								| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}


void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
	}
}


/* ======================================================================
 * RTOS 任务划分（优先级按"实时性要求"从高到低，理性分配）：
 *
 *   Task_Control    10ms  姿态解算 + 角度环    优先级 4（控制核心，最实时）
 *   Task_Speed      50ms  速度环 + 转向环      优先级 3
 *   Task_Distance   100ms 超声波测距 + 距离环   优先级 2
 *   Task_OLED       100ms OLED 刷新            优先级 2
 *   Task_Bluetooth  50ms  蓝牙调参解析         优先级 1（最不实时）
 *
 * 说明：姿态/角度环是自平衡的关键，必须最高优先级且周期最短；
 *       速度环、距离环属于外环，周期可放宽；OLED、蓝牙纯属锦上添花。
 *       所有任务均用 vTaskDelayUntil 绝对延时，周期恒定不受任务体耗时影响。
 * ====================================================================== */

static void Task_Control(void *argument)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	(void)argument;

	for (;;)
	{
		/**************** LED状态 ****************/
		if (RunFlag)
		{
			LED_ON();
		}
		else
		{
			LED_OFF();
		}

		/**************** 按键 ****************/
		KeyNum = Key_GetNum();

		if (KeyNum == 1)
		{
			if (RunFlag == 0)
			{
				/* 启动：初始化各 PID */
				PID_Init(&AnglePID);
				PID_Init(&SpeedPID);
				PID_Init(&TurnPID);
				PID_Init(&DistancePID);
				RunFlag = 1;
			}
			else
			{
				/* 停止 */
				RunFlag = 0;
				Motor_SetSpeed(1, 0);
				Motor_SetSpeed(2, 0);
			}
		}

		/**************** MPU姿态解算 + 角度环 10ms ****************/
		MPU6050_Update();
		Angle_Update();

		if (RunFlag)
		{
			Angle_PID_Control();
		}
		else
		{
			Motor_SetSpeed(1, 0);
			Motor_SetSpeed(2, 0);
		}

		/* 绝对延时：固定 10ms 周期 */
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
	}
}

static void Task_Speed(void *argument)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	(void)argument;

	for (;;)
	{
		/**************** 速度 + 转向环 50ms ****************/
		if (RunFlag)
		{
			Speed_PID_Control();
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
	}
}

static void Task_Distance(void *argument)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	(void)argument;

	for (;;)
	{
		/**************** 超声波控距 100ms ****************/
		if (RunFlag)
		{
			Distance_PID_Control();
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
	}
}

static void Task_OLED(void *argument)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	(void)argument;

	for (;;)
	{
		/**************** OLED 刷新 100ms ****************/
		OLED_Show();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
	}
}

static void Task_Bluetooth(void *argument)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	(void)argument;

	for (;;)
	{
		/**************** 蓝牙调参解析 50ms ****************/
		if (BlueSerial_RxFlag == 1)
		{
			char *Tag = strtok(BlueSerial_RxPacket, ",");

			if (strcmp(Tag, "slider") == 0)
			{
				char *Name = strtok(NULL, ",");
				char *Value = strtok(NULL, ",");

				if (strcmp(Name, "AngleKp") == 0)
				{
					AnglePID.Kp = atof(Value);
				}
				else if (strcmp(Name, "AngleKi") == 0)
				{
					AnglePID.Ki = atof(Value);
				}
				else if (strcmp(Name, "AngleKd") == 0)
				{
					AnglePID.Kd = atof(Value);
				}
				else if (strcmp(Name, "SpeedKp") == 0)
				{
					SpeedPID.Kp = atof(Value);
				}
				else if (strcmp(Name, "SpeedKi") == 0)
				{
					SpeedPID.Ki = atof(Value);
				}
				else if (strcmp(Name, "SpeedKd") == 0)
				{
					SpeedPID.Kd = atof(Value);
				}
				else if (strcmp(Name, "TurnKp") == 0)
				{
					TurnPID.Kp = atof(Value);
				}
				else if (strcmp(Name, "TurnKi") == 0)
				{
					TurnPID.Ki = atof(Value);
				}
				else if (strcmp(Name, "TurnKd") == 0)
				{
					TurnPID.Kd = atof(Value);
				}
				else if (strcmp(Name, "DistKp") == 0)
				{
					DistancePID.Kp = atof(Value);
				}
				else if (strcmp(Name, "DistKi") == 0)
				{
					DistancePID.Ki = atof(Value);
				}
				else if (strcmp(Name, "DistKd") == 0)
				{
					DistancePID.Kd = atof(Value);
				}
				else if (strcmp(Name, "DistTarget") == 0)
				{
					DistancePID.Target = atof(Value);
				}
				else if (strcmp(Name, "Angleoffset") == 0)
				{
					AnglePID.OutOffset = atof(Value);
				}
			}

			BlueSerial_RxFlag = 0;
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
	}
}


/******************** FreeRTOS 错误钩子 ********************/


void vApplicationMallocFailedHook(void)
{
	/* 内存分配失败：停机 */
	Error_Handler();
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	(void)xTask;
	(void)pcTaskName;
	/* 任务栈溢出：停机 */
	Error_Handler();
}


int main(void)
{

	HAL_Init();

	SystemClock_Config();

	OLED_Init();

	MPU_Init();

	BlueSerial_Init();

	LED_Init();

	Key_Init();

	Motor_Init();

	Encoder_Init();

	HC_SR04_Init();

	USB_Init();

	Timer_Init();

	/* 创建 RTOS 任务：按实时性分配优先级（详见任务函数上方的说明） */
	xTaskCreate(Task_Control,   "Control",   256, NULL, 4, NULL);
	xTaskCreate(Task_Speed,      "Speed",     256, NULL, 3, NULL);
	xTaskCreate(Task_Distance,   "Distance",  256, NULL, 2, NULL);
	xTaskCreate(Task_OLED,       "OLED",      256, NULL, 2, NULL);
	xTaskCreate(Task_Bluetooth,  "Bluetooth", 256, NULL, 1, NULL);

#if RTOS_DEMO_ENABLE
	/* FreeRTOS 学习演示：创建各知识点演示任务 */
	RTOS_Demo_Start();
#endif

	/* 启动 FreeRTOS 调度器（不再返回） */
	vTaskStartScheduler();

	/* 正常情况下不会执行到这里 */
	while (1)
	{
	}
}


/******************** MPU6050读取 ********************/


void MPU6050_Update(void)
{
	MPU_Read_All(
	&AX,
	&AY,
	&AZ,
	&GX,
	&GY,
	&GZ
	);



	// 陀螺零偏

	GY -= 16;

}




/******************** 姿态解算 ********************/


void Angle_Update(void)
{


	/******** 加速度角 ********/


	AngleAcc =
	-atan2(AX,AZ)
	/
	3.14159f
	*
	180.0f;



	/******** 一级低通 ********/


	AngleAcc_Filter =
	0.8f * AngleAcc_Filter
	+
	0.2f * AngleAcc;





	/******** 陀螺积分 ********/


	AngleGyro =
	Angle
	+
	GY
	/
	32768.0f
	*
	2000.0f
	*
	0.01f;





	/******** 动态互补滤波 ********/


	if(fabs(GY) > 300)
	{

		Alpha = 0.005f;

	}

	else
	{

		Alpha = 0.03f;

	}





	Angle =
	Alpha * AngleAcc_Filter
	+
	(1-Alpha) * AngleGyro;





	/******** 安全保护 ********/


	if(Angle > 50 || Angle < -50)
	{

		RunFlag = 0;

	}



}


void Angle_PID_Control(void)
{

	AnglePID.Actual = Angle;


	PID_Update(&AnglePID);



	// 角度环输出PWM

	AvePWM = -AnglePID.Out;



	// 差速叠加

	LeftPWM = AvePWM + DifPWM / 2;

	RightPWM = AvePWM - DifPWM / 2;



	// PWM限幅

	if(LeftPWM > 100)
	{
		LeftPWM = 100;
	}

	else if(LeftPWM < -100)
	{
		LeftPWM = -100;
	}



	if(RightPWM > 100)
	{
		RightPWM = 100;
	}

	else if(RightPWM < -100)
	{
		RightPWM = -100;
	}




	Motor_SetSpeed(1,LeftPWM);

	Motor_SetSpeed(2,RightPWM);


}





void Speed_PID_Control(void)
{


	/******** 编码器速度 ********/


	LeftSpeed =
	Encoder_Get(1)
	/
	44.0f
	/
	0.05f
	/
	9.27666f;



	RightSpeed =
	Encoder_Get(2)
	/
	44.0f
	/
	0.05f
	/
	9.27666f;




	AveSpeed =
	(LeftSpeed + RightSpeed)
	/
	2.0f;



	DifSpeed =
	LeftSpeed - RightSpeed;






	/******** 速度环 ********/


	SpeedPID.Actual = AveSpeed;


	PID_Update(&SpeedPID);



	//速度环输出角度目标

	AnglePID.Target = SpeedPID.Out;





	/******** 转向环 ********/


	TurnPID.Actual = DifSpeed;


	PID_Update(&TurnPID);



	DifPWM = TurnPID.Out;


}





void Distance_PID_Control(void)
{


	/******** 超声波测距 ********/


	Distance = HC_SR04_GetDistance();


	if(Distance < 0)
	{

		return;

	}



	/******** 距离环 ********/


	DistancePID.Actual = Distance;

	PID_Update(&DistancePID);


	//距离环输出为速度环的目标速度

	SpeedPID.Target = DistancePID.Out;

}




void OLED_Show(void)
{


	OLED_Clear();



	OLED_Printf(0,0,OLED_6X8,
	"Angle");



	OLED_Printf(0,8,OLED_6X8,
	"P:%05.2f",
	AnglePID.Kp);



	OLED_Printf(0,16,OLED_6X8,
	"I:%05.2f",
	AnglePID.Ki);



	OLED_Printf(0,24,OLED_6X8,
	"D:%05.2f",
	AnglePID.Kd);



	OLED_Printf(0,32,OLED_6X8,
	"T:%+05.1f",
	AnglePID.Target);



	OLED_Printf(0,40,OLED_6X8,
	"A:%+05.1f",
	Angle);



	OLED_Printf(0,48,OLED_6X8,
	"O:%+05.0f",
	AnglePID.Out);



	OLED_Printf(0,56,OLED_6X8,
	"GY:%+05d",
	GY);




	OLED_Printf(50,0,OLED_6X8,
	"Speed");



	OLED_Printf(50,8,OLED_6X8,
	"%05.2f",
	SpeedPID.Kp);



	OLED_Printf(50,16,OLED_6X8,
	"%05.2f",
	SpeedPID.Ki);



	OLED_Printf(50,24,OLED_6X8,
	"%05.2f",
	SpeedPID.Kd);



	OLED_Printf(50,32,OLED_6X8,
	"%+05.1f",
	SpeedPID.Target);



	OLED_Printf(50,40,OLED_6X8,
	"%+05.1f",
	AveSpeed);



	OLED_Printf(50,48,OLED_6X8,
	"%+05.0f",
	SpeedPID.Out);


	OLED_Printf(50,56,OLED_6X8,
	"D:%04.0f",
	Distance);




	OLED_Printf(88,0,OLED_6X8,
	"Turn");



	OLED_Printf(88,8,OLED_6X8,
	"%05.2f",
	TurnPID.Kp);



	OLED_Printf(88,16,OLED_6X8,
	"%05.2f",
	TurnPID.Ki);



	OLED_Printf(88,24,OLED_6X8,
	"%05.2f",
	TurnPID.Kd);



	OLED_Printf(88,32,OLED_6X8,
	"%+05.1f",
	TurnPID.Target);



	OLED_Printf(88,40,OLED_6X8,
	"%+05.1f",
	DifSpeed);



	OLED_Printf(88,48,OLED_6X8,
	"%+05.0f",
	TurnPID.Out);



	OLED_Printf(88,56,OLED_6X8,
	"%+04.0f",
	AnglePID.OutOffset);



	OLED_Update();

}

/******************** 断言失败处理 (USE_FULL_ASSERT) ********************/


void assert_failed(uint8_t *file, uint32_t line)
{
	(void)file;
	(void)line;
	Error_Handler();
}
