/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oledapp.h"
#include "MPU6050.h"
#include "robot.h"
#include "motor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for DefaultTask */
osThreadId_t DefaultTaskHandle;
const osThreadAttr_t DefaultTask_attributes = {
  .name = "DefaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for BalanceTask */
osThreadId_t BalanceTaskHandle;
const osThreadAttr_t BalanceTask_attributes = {
  .name = "BalanceTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for MpuCalcTask */
osThreadId_t MpuCalcTaskHandle;
const osThreadAttr_t MpuCalcTask_attributes = {
  .name = "MpuCalcTask",
  .stack_size = 200 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CommandTask */
osThreadId_t CommandTaskHandle;
const osThreadAttr_t CommandTask_attributes = {
  .name = "CommandTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal7,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern robot PHC;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartBalanceTask(void *argument);
void StartMpuCalcTask(void *argument);
void StartCommandTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of DefaultTask */
  DefaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &DefaultTask_attributes);

  /* creation of BalanceTask */
  BalanceTaskHandle = osThreadNew(StartBalanceTask, NULL, &BalanceTask_attributes);

  /* creation of MpuCalcTask */
  MpuCalcTaskHandle = osThreadNew(StartMpuCalcTask, NULL, &MpuCalcTask_attributes);

  /* creation of CommandTask */
  CommandTaskHandle = osThreadNew(StartCommandTask, NULL, &CommandTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the DefaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  portTickType currentTimeDefault;
  currentTimeDefault = xTaskGetTickCount();
  /* Infinite loop */
  for (;;)
  {
    OledAppLoop(&PHC.oled);
    vTaskDelayUntil(&currentTimeDefault, 100);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartBalanceTask */
/**
 * @brief Function implementing the BalanceTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartBalanceTask */
void StartBalanceTask(void *argument)
{
  /* USER CODE BEGIN StartBalanceTask */
  portTickType currentTimeBalance;
  currentTimeBalance = xTaskGetTickCount();
  /* Infinite loop */
  for (;;)
  {
    motor_getvelocity(&PHC.motor_left);
    motor_getvelocity(&PHC.motor_right);
    default_loop();
    vTaskDelayUntil(&currentTimeBalance, 5);
  }
  /* USER CODE END StartBalanceTask */
}

/* USER CODE BEGIN Header_StartMpuCalcTask */
/**
 * @brief Function implementing the MpuCalcTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMpuCalcTask */
void StartMpuCalcTask(void *argument)
{
  /* USER CODE BEGIN StartMpuCalcTask */
  portTickType currentTimempu;
  currentTimempu = xTaskGetTickCount();
  /* Infinite loop */
  for (;;)
  {
    MPU6050_Update(&PHC.imu);
    vTaskDelayUntil(&currentTimempu, 5);
  }
  /* USER CODE END StartMpuCalcTask */
}

/* USER CODE BEGIN Header_StartCommandTask */
/**
 * @brief Function implementing the CommandTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommandTask */
void StartCommandTask(void *argument)
{
  /* USER CODE BEGIN StartCommandTask */
  portTickType currentTimecommand;
  currentTimecommand = xTaskGetTickCount();
  /* Infinite loop */
  for (;;)
  {
    Robot_Command();
    vTaskDelayUntil(&currentTimecommand, 5);
  }
  /* USER CODE END StartCommandTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

