/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "unistd.h"

#include "glove_status_codes.h"
#include "hand.h"
#include "queue.h"
#include "scheduler.h"
#include "serial.h"
#include "sm.h"
#include "TCA9548A.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TIMEOUT 100
#define I2C_TIMEOUT 1000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
bool gfEnablePrintf = true;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
glove_status_t queue_test();
glove_status_t scheduler_tests();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  glove_status_t status = GLOVE_STATUS_OK;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(200);
  printf("\r\nHello world!!\r\n");
  // while (1) {
  //   printf("Hello world!!!\r\n");
  //   HAL_Delay(200);
  // }

  // blinky
  // while (1)
  // {
  //   HAL_GPIO_TogglePin(I2C_MUX_RST_GPIO_Port, I2C_MUX_RST_Pin);
  //   HAL_Delay(100);
  // }

  // initialize the serial interface wrapper
  if ((status = Serial_Init(&huart2)))
  {
    printf("Serial interface init failed, status=%X\r\n", status);
  }

  // loop printf
  // while (1)
  // {
  //   printf("Hello world!!!\r\n");
  //   HAL_Delay(100);
  // }


  // initialize the I2C mux
  if ( (status = I2CMux_Init(&hi2c1)) )
  {
    printf("I2C mux init failed, status=%X\r\n", status);
  }

  // // select the first bus
  // if ( (status = I2CMux_Select(1)) )
  // {
  //   printf("I2C mux select failed, status=%X\r\n", status);
  // }

  // initialize Hand 
  if ( (status = Hand_Init(&hi2c1)) )
  {
    printf("Hand init failed, status=%x\r\n", status);
  }

  // state machine
  if ((status = SM_Init()))
  {
    printf("State machine init failed\r\n");
  }

  // scheduler
  if ((status = Scheduler_Init()))
  {
    printf("Scheduler init failed\r\n");
  }

  // status = queue_test();
  // if (GLOVE_STATUS_OK != status)
  // {
  //   printf("Queue tests failed!, status = %X\r\n", status);
  // }
  // else
  // {
  //   printf("Queue tests passed!\r\n");
  // }

  // status = scheduler_tests();
  // if (GLOVE_STATUS_OK != status)
  // {
  //   printf("Scheduler tests failed!, status = %X\r\n", status);
  // }
  // else
  // {
  //   printf("Scheduler tests passed!\r\n");
  // }
  

  // if ((status = IMU_ReadAll(&motionData)))
  // {
  //   printf("IMU read all failed\r\n");
  // }

  printf("Main pre-loop done\r\n");

  HAL_Delay(200);

  gfEnablePrintf = false;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // SM_PostEvent(EVENT_START_TRANSFERRING);
  while (1)
  {
    SM_Tick();
    Scheduler_Tick();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00702991;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
  huart2.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(I2C_MUX_RST_GPIO_Port, I2C_MUX_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin I2C_MUX_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|I2C_MUX_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

int _write(int file, char *data, int len)
{
    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
    {
        return -1;
    }
    if (gfEnablePrintf)
    {
	    return HAL_UART_Transmit(&huart2, (uint8_t *)data, len, UART_TIMEOUT);
    }
    return 0;
}

// ****************** Tests *************************************
// TODO: find a better place to put these tests
void sm_test()
{
  // sm test
  SM_Init();
  SM_Tick();
  SM_PostEvent(EVENT_START_TRANSFERRING);
  SM_Tick();
  SM_PostEvent(EVENT_STOP_TRANSFERRING);
  SM_Tick();
  SM_Tick();
  SM_PostEvent(EVENT_NONE);
  SM_Tick();
}

glove_status_t queue_test()
{
  queue_t testQueue = {0};
  glove_status_t status = GLOVE_STATUS_OK;
  uint8_t i = 0;
  uint32_t * item = NULL;
  uint32_t items[16] = {0};

  // initialize to numbers 0 to 15
  for (uint8_t i = 0; i < 16; ++i)
  {
    items[i] = i;
  }

  // enqueue 4 items and then dequeue them
  printf("%s simple enqueue dequeue\r\n", __FUNCTION__);
  status = Queue_Init(&testQueue, 4);
  CHECK_STATUS_OK_RET(status);
  for (i = 0; i < 4; ++i)
  {
    status = Queue_Enqueue(&testQueue, items + i);
    CHECK_STATUS_OK_RET(status);
  }
  for (i = 0; i < 4; ++i)
  {
    item = (uint32_t *)Queue_Dequeue(&testQueue);
    CHECK_NULL_RET(item);
    if (i != *item)
    {
      printf("Test fail: %s:%d, i=%d, *item=%lu\r\n", __FUNCTION__, __LINE__, i, *item);
      return GLOVE_STATUS_FAIL;
    }
  }
  
  // enqueue 8 items, dequeing right after enqueue
  printf("%s enqueue immediate dequeue\r\n", __FUNCTION__);
  status = Queue_Init(&testQueue, 4);
  CHECK_STATUS_OK_RET(status);
  for (i = 0 ; i < 8; ++i)
  {
    status = Queue_Enqueue(&testQueue, items + i);
    CHECK_STATUS_OK_RET(status);
    item = (uint32_t *)Queue_Dequeue(&testQueue);
    CHECK_NULL_RET(item);
    if (i != *item)
    {
      printf("Test fail: %s:%d\r\n", __FUNCTION__, __LINE__);
      return GLOVE_STATUS_FAIL;
    }
  }

  // enqueue more 9 items in a queue, and make sure that only the last 8 are dequeued
  printf("%s enqueue 1 extra \r\n", __FUNCTION__);
  status = Queue_Init(&testQueue, 8);
  for (i = 0; i < 9; ++i)
  {
    status = Queue_Enqueue(&testQueue, items + i);
    CHECK_STATUS_OK_RET(status)
  }
  for (i = 0; i < 8; ++i)
  {
    item = (uint32_t *)Queue_Dequeue(&testQueue);
    CHECK_NULL_RET(item);

    if (i + 1 != *item)
    {
      printf("Test fail: %s:%d\r\n", __FUNCTION__, __LINE__);
      return GLOVE_STATUS_FAIL;
    }
  }

  // enqueue 16 items in a 8-item queue, and make sure that only the last 8 are dequeued
  printf("%s enqueue 2x extra \r\n", __FUNCTION__);  
  status = Queue_Init(&testQueue, 8);
  for (i = 0; i < 16; ++i)
  {
    status = Queue_Enqueue(&testQueue, items + i);
    CHECK_STATUS_OK_RET(status);
  }
  for (i = 0; i < 8; ++i)
  {
    item = (uint32_t *)Queue_Dequeue(&testQueue);
    CHECK_NULL_RET(item);
    if (i + 8 != *item)
    {
      printf("Test fail: %s:%d\r\n", __FUNCTION__, __LINE__);
      return GLOVE_STATUS_FAIL;
    }
  }

  // try to dequeue from an empty queue, should be null
  printf("%s dequeue empty \r\n", __FUNCTION__);
  status = Queue_Init(&testQueue, 4);
  CHECK_STATUS_OK_RET(status);
  item = Queue_Dequeue(&testQueue);
  if (item)
  {
    printf("Test fail: %s:%d\r\n", __FUNCTION__, __LINE__);
    return GLOVE_STATUS_FAIL;
  }
  return GLOVE_STATUS_OK;
}

glove_status_t empty_task()
{
  return GLOVE_STATUS_OK;
}
task_t emptyTask = 
{
  .pTaskFn = &empty_task,
  .name = "Empty Task"
};

uint32_t test_counter = 0;
glove_status_t test_task()
{
  ++test_counter;
  return GLOVE_STATUS_OK;
}
task_t testTask = 
{
  .pTaskFn = &test_task,
  .name = "Test task"
};

glove_status_t scheduler_tests()
{
  glove_status_t status = GLOVE_STATUS_OK;
  uint32_t i = 0;

  status = Scheduler_Init();
  CHECK_STATUS_OK_RET(status);

  // add an empty task and tick once
  status = Scheduler_AddTask(&emptyTask);
  CHECK_STATUS_OK_RET(status);
  status = Scheduler_Tick();

  // add one of the test_tasks and tick once
  status = Scheduler_AddTask(&testTask);
  CHECK_STATUS_OK_RET(status);
  status = Scheduler_Tick();
  if (test_counter != 1)
  {
    printf("test fail %s:%d, test_counter = %lu\r\n", __FUNCTION__, __LINE__, test_counter);
    return GLOVE_STATUS_FAIL;
  }

  // add multiple test_tasks and tick until they should all be done
  test_counter = 0;
  for (i = 0; i < SCHEDULER_MAX_NUM_TASKS; ++i)
  {
    status = Scheduler_AddTask(&testTask);
    CHECK_STATUS_OK_RET(status);
  }
  for (i = 0; i < SCHEDULER_MAX_NUM_TASKS; ++i)
  {
    Scheduler_Tick();
    CHECK_STATUS_OK_RET(status);
  }
  if (test_counter != SCHEDULER_MAX_NUM_TASKS)
  {
    printf("test fail %s:%d, test_counter = %lu\r\n", __FUNCTION__, __LINE__, test_counter);
    return GLOVE_STATUS_FAIL;
  }
  
  return GLOVE_STATUS_OK;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
