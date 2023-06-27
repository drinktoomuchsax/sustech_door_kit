/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "FRM_Protocol.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

uint8_t g_fUART1_Byte;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

volatile int is_powerdown = 0;

P_RECV_MSG p_function_1;
P_RECV_MSG p_function_2;
P_RECV_MSG p_function_3;

static RECV_MSG function;
volatile uint8_t IT_Button = 0;

USER user;
static int times = 0;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

extern void MSG_Send(uint8_t *sendData, uint32_t sendLen)
{

	HAL_UART_Transmit(&huart1, sendData, sendLen, 1000);
	HAL_UART_Receive_IT(&huart1, &g_fUART1_Byte, 1 );
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){}

void USART1_IRQHandler(void)
{

  /* USER CODE BEGIN USART1_IRQn 0 */
  /* data storage*/
	//old
	
	
  /* USER CODE END USART1_IRQn 0 */
 	
  /* USER CODE BEGIN USART1_IRQn 1 */
	HAL_UART_IRQHandler(&huart1);
  //fresh
	HAL_UART_Receive_IT(&huart1, &g_fUART1_Byte, 1);
	Receive_Interruptiion(g_fUART1_Byte, &function);
  /* USER CODE END USART1_IRQn 1 */
}

void EXTI2_3_IRQHandler(uint16_t GPIO_Pin)
{
  /* USER CODE BEGIN EXTI2_3_IRQn 0 */

  /* USER CODE END EXTI2_3_IRQn 0 */
  if(GPIO_Pin == GPIO_PIN_2)
  {
  	IT_Button = ENROLLMENT_BUTTON;
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  }
  else if(GPIO_Pin == GPIO_PIN_3)
  {
  	IT_Button = DELETE_ALL_BUTTON;
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
  }
  HAL_ResumeTick();
  times = 0;
  /* USER CODE BEGIN EXTI2_3_IRQn 1 */

  /* USER CODE END EXTI2_3_IRQn 1 */
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /*
  ML_API_GetFirmwareVersion(&function);
	ML_API_GetUUID(&function);*/
  /*
 function = *p_function_1;
  ML_API_DeleteAll(&function);
   ML_API_DeleteID(2, &function);*/
  
 /*
  ML_API_Match(&user, &function);
    */
/*
	user.isAdm = 0x00;
	char name[32] = "micheng";
	char* str_dir[] = {"front", "up", "down", "left", "right"};
	memcpy(user.name, name, 32);
	for (int i = 0; i < 5 ; i++)
	{
		printf("pls look %s\n", str_dir[i]);
		int result = ML_API_Enrollment(&function, &user, i, 5);
		if(result == 0)
		{
			printf("enroll timeout!\n");
			break;
		}
		else if (result == 2)
		{
			printf("enrollment success, the user ");
			for (int a = 0; a< sizeof(user.name)&&user.name[a] != 0x00; a++)
				printf("%c", user.name[a]);
			printf("'s id: %d\n", user.id[0]+user.id[1]);
			break;
		}		
	}
*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /*
  HAL_TIM_Base_Start_IT(&htim3);
  */
  function = *p_function_1;
  ML_API_DeleteAll(&function);/*
  
  HAL_TIM_Base_Start_IT(&htim3);
  while(!is_powerdown);
  function = *p_function_2;
   ML_API_PowerDown(&function);
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  HAL_ResumeTick();
  
  while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3))
	  is_powerdown = 0;*/
 
  int result = 0;

  while(1)
  {
	  while (!IT_Button)
	  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
			function = *p_function_1;
			result = ML_API_Match(&user, &function);
			if (result == -1)
				printf("cmd with no respond!!");
			else if(result == 1)
			{
				printf("match success, the user's id: %d%d, the user's name:", user.id[0], user.id[1]);
				for (int i = 0; i<sizeof(user.name); i++)
				{  
					if(user.name[i] == 0x00)
						break;
					else
						printf("%c",user.name[i]);
				}
				printf("\r\n");
			}
			else
				times ++;
			if (times == 3)
			{
				HAL_SuspendTick();
				HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			}
	  }
	  switch(IT_Button)
	  {
		case ENROLLMENT_BUTTON:
			user.isAdm = 0x00;
			function = *p_function_2;
	  		ML_API_DeleteAll(&function);
			char name[32] = "micheng";
			char* str_dir[] = {"front", "up", "down", "left", "right"};
			memcpy(user.name, name, 32);
			for (int i = 0; i < 5 ; i++)
			{
				printf("pls look %s\n", str_dir[i]);
				int result = ML_API_Enrollment(&function, &user, i, 5);
				if(result == 0)
				{
					printf("enroll timeout!\n");
					break;
				}
				else if (result == 2)
				{
					printf("enrollment success, the user ");
					for (int a = 0; a< sizeof(user.name)&&user.name[a] != 0x00; a++)
						printf("%c", user.name[a]);
					printf("'s id: %d\n", user.id[0]+user.id[1]);
					break;
				}		
			}
			IT_Button = 0;
			break;

		case DELETE_ALL_BUTTON:
			function = *p_function_3;
	  		ML_API_DeleteAll(&function);
			IT_Button = 0;
			break;
			
	  }
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart1.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pins : PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
	uint8_t temp_ch = ch;
	HAL_UART_Transmit( &huart2, &temp_ch, 1, 100);
	return (ch);
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
  __disable_irq();
  while (1)
  {
  }
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
