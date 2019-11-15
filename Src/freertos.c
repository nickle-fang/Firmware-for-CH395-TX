/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "NRF24L01.h"
#include "CH395.h"
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern uint8_t TX_Packet[25];
extern uint8_t RX_Packet[25];
extern uint8_t rx_success;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern uint8_t IPShow[];
extern uint8_t RemoteIpShow[20];
extern uint8_t RemoteIpLen;
extern UART_HandleTypeDef huart3;
extern uint8_t destAddr[];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void RX_Task(void const * argument);
void UDP_REV_Task(void const * argument);
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

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
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	osThreadDef(RXTask, RX_Task, osPriorityRealtime, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(RXTask), NULL);
	
	osThreadDef(UDP_Task, UDP_REV_Task, osPriorityHigh, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(UDP_Task), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
    
  /* USER CODE BEGIN StartDefaultTask */
	//OLED_ShowStr(0, 0, "ZJUNlict Trans",2);
	OLED_ShowStr(0, 0, IPShow, 1);
	OLED_ShowStr(0, 2, RemoteIpShow, 1);
	//OLED_ShowChar(0, 0, );
	
  /* Infinite loop */
  for(;;)
  {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		//OLED_ShowStr(0, 2, RemoteIpShow, 1);
		//HAL_UART_Transmit(&huart3, destAddr, 4, 0xFF);
		//HAL_UART_Transmit(&huart3, RemoteIpShow, 20, 0xFF);

    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void RX_Task(void const * argument)
{
	while(1)
	{
		if ( NRF24L01_RxPacket(RX_Packet) == 0){
			//CH395UDPSendTo(RX_Packet, 25, destAddr, 1030, 1);
			CH395SendData( 0, RX_Packet, 25 );  /*向发送缓冲区写数据*/
			rx_success ++;
			if (rx_success > 32){
					rx_success = 0;
					HAL_GPIO_TogglePin(LED_RX_GPIO_Port, LED_RX_Pin);
			}
		}
		osDelay(1);
	}
}


void UDP_REV_Task(void const * argument)
{
	while(1)
	{
		if (HAL_GPIO_ReadPin(INT_GPIO_Port, INT_Pin) == 0 ){
			CH395GlobalInterrupt();
		}
		osDelay(3);
	}
}
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
