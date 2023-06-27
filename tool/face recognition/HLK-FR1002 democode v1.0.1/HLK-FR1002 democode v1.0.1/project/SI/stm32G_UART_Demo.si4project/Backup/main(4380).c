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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GET_HEAD (0)
#define GET_LENGTH (1)
#define GET_CMD (2)
#define GET_STATE (3)
#define GET_DATA (4)
#define GET_PARITY (5)
#define END_RECIEVE (6)
#define MAX_SIZE (64)
#define NORM_CMD_TIME_OUT (5)

uint8_t ring_buff[100];
uint8_t g_fUART1_Byte;
uint8_t uart_get_flag = 0;
uint8_t head_bytes[2] = {0xEF, 0xAA};
uint8_t data_recieve_finished = 0;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint32_t ring_buff_index = 0;
uint32_t current_step = 0;
uint32_t length;

typedef struct
{
	uint8_t data_length[2];
	uint8_t cmd;
	uint8_t state;
	uint8_t data[MAX_SIZE];
	uint8_t parity;
}RECV_MSG, *P_RECV_MSG;

typedef struct
{
	uint32_t isAdm;
	uint8_t name[32];
	uint8_t id[2];
}USER, *P_USER;

RECV_MSG function;
USER user;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
uint8_t VerifyParity(uint8_t data, P_RECV_MSG p_recv_msg);
void State_of_DeleteAll(void);
void State_of_DeleteID(uint8_t id);
int State_of_Match(P_USER p_user);
int State_of_Enrollment(P_USER p_user, uint8_t dir, uint8_t enrollment_pattern);
void Version_Send(uint8_t * data, uint32_t data_len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Delay_us(uint32_t delay_us)
{    
  volatile unsigned int num;
  volatile unsigned int t;
 
  
  for (num = 0; num < delay_us; num++)
  {
    t = 11;
    while (t != 0)
    {
      t--;
    }
  }
}

void Delay_ms(uint16_t delay_ms)
{    
  volatile unsigned int num;
  for (num = 0; num < delay_ms; num++)
  {
    Delay_us(1000);
  }
}

unsigned char GetParityCheck(uint8_t *p, int len)
{
	uint8_t nParityCheck = 0;
	for (int i = 2; i < len - 1; ++i)
	{
		nParityCheck ^= p[i];
	}

	return nParityCheck;
}

void DeleteID(UART_HandleTypeDef huart, uint8_t id)
{
	uint8_t pCmd1[8] = { 0xEF, 0xAA, 0x20, 0x00, 0x02 };
	function.cmd = 0x20;
	pCmd1[5] = (id&0xff00)>>8;//取高8位
	pCmd1[6] = id&0xff;//取低8位
	pCmd1[7] = GetParityCheck(pCmd1, sizeof(pCmd1));
	uint8_t nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	HAL_UART_Transmit(&huart, pCmd1, 8, 1000);
	HAL_UART_Receive_IT(&huart, &g_fUART1_Byte, 1 );
	
	for(uint8_t i = 0; i < nCycleTime; ++i)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);			
	State_of_DeleteID(id);
	data_recieve_finished = 0;
}


void DeleteAll(UART_HandleTypeDef huart) 
{
	uint8_t pCmd1[6] = { 0xEF, 0xAA, 0x21, 0x00, 0x00, 0x21 };
	function.cmd = 0x21;
	HAL_UART_Transmit(&huart, pCmd1, 6, 1000);
	HAL_UART_Receive_IT(&huart, &g_fUART1_Byte, 1 );
	while(!data_recieve_finished);
	State_of_DeleteAll();
	data_recieve_finished = 0;
}

void GetFirmwareVersion(UART_HandleTypeDef huart)
{
	uint8_t pCmd1[8] = { 0xEF, 0xAA, 0x30, 0x00, 0x00, 0x30};
	function.cmd = 0x30;
	HAL_UART_Transmit(&huart, pCmd1, 6, 1000);
	HAL_UART_Receive_IT(&huart, &g_fUART1_Byte, 1 );
	while(!data_recieve_finished);
	Version_Send(function.data, function.data_length[0]*256+function.data_length[1]-2);
	data_recieve_finished = 0;
}

int Enrollment(UART_HandleTypeDef huart, P_USER p_user, uint8_t dir, uint8_t enrollment_pattern)
{
	uint8_t pEnrollCmd[] = { 0xEF, 0xAA, 0x26, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x10,
		   0x00, 0x00, 0x00, 0x1E };
	function.cmd = 0x26;
	pEnrollCmd[5] = p_user->isAdm;
	
	if (dir == 4 || enrollment_pattern == 1)
		memcpy(pEnrollCmd + 6, p_user->name, 32);
	
	uint8_t face_dir[5] = { 0x01, 0x10, 0x08, 0x04, 0x02 };
	uint8_t pAns[5] = { 0x01, 0x11, 0x19, 0x1D, 0x1F };
	
	pEnrollCmd[38] = face_dir[dir];

	if (enrollment_pattern == 5) {
		pEnrollCmd[39] = 0x00;
	}
	else if (enrollment_pattern == 1) {
		pEnrollCmd[39] = 0x01;
	}
	
	uint8_t nTimeOut = 0x0A;
	int nWaiteTime = 5;
	
	pEnrollCmd[41] = nTimeOut;
	pEnrollCmd[45] = GetParityCheck(pEnrollCmd, sizeof(pEnrollCmd));

	int nCycleTime = nTimeOut * 1000 / nWaiteTime;

	HAL_UART_Transmit(&huart, pEnrollCmd, 46, 1000);
	HAL_UART_Receive_IT(&huart, &g_fUART1_Byte, 1);
	//----------------------
	int result;
	int t = 0;
	int enroll_success = 0;
	int enrolling = 0;
	if (enrollment_pattern == 1)
	{
		while (1)
		{
			for (; t < nCycleTime; ++t)
			{
				if(data_recieve_finished)
				{
					t = 0;
					break;
				}
				else
					Delay_ms(nWaiteTime);
			}

			if(t>=nCycleTime)
			{
				printf("enrollment cmd sending with no respond!");
				enrolling = 0;
				enroll_success = 0;
				break;
			}

			result = State_of_Enrollment(p_user, pAns[dir], enrollment_pattern);
			if(result != -1)
			{
				if(result == 0)
				{
					printf("enrollment timeout!");
					enrolling = 0;
					enroll_success = 0;
					break;
				}
				else
				{
					enrolling = 0;
					enroll_success = 1;
					break;
				}
			}
			data_recieve_finished = 0;
		}
	}
	else
	{
		while (1)
		{
			for (; t < nCycleTime; ++t)
			{
				if(data_recieve_finished)
					break;
				else
					Delay_ms(nWaiteTime);
			}

			if(t>=nCycleTime)
			{
				printf("enrollment cmd sending with no respond!");
				enrolling = 0;
				enroll_success = 0;
				break;
			}

			result = State_of_Enrollment(p_user, pAns[dir], enrollment_pattern);
			
			if(result != -1)
			{
				if(result == 0)
				{
					printf("enrollment timeout!");
					enrolling = 0;
					enroll_success = 0;
					break;
				}
				else
				{
					if(dir == 4)
					{
						enrolling = 0;
						enroll_success = 1;
						break;
					}
					else
					{
						enrolling = 1;
						enroll_success = 1;
						break;
					}
				}
			}
			data_recieve_finished = 0;
		}
	}
	data_recieve_finished = 0;
	if(enrolling == 0 && enroll_success== 0)
		return 0;//timeout feedback
	else if(enrolling == 1 && enroll_success == 1)
		return 1;//next dir feedback
	else if(enrolling == 0 && enroll_success == 1)
		return 2;//success feedback
	else
		return -1;
}

void Match(UART_HandleTypeDef huart, P_USER p_user)
{
	uint8_t pCmd1[] = { 0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x03, 0x13 };
	function.cmd = 0x12;

	HAL_UART_Transmit(&huart, pCmd1, 8, 1000);
	HAL_UART_Receive_IT(&huart, &g_fUART1_Byte, 1);
	int result;
	int nCycleTime = (0x03+1) * 1000 / NORM_CMD_TIME_OUT;
	int t = 0;
	while (1){
		for (; t < nCycleTime; ++t)
			{
				if(data_recieve_finished)
					break;
				else
					Delay_ms(NORM_CMD_TIME_OUT);
			}

		if(t>=nCycleTime)
		{
			printf("match cmd sending with no respond!");
			t=0;
			break;
		}

		result = State_of_Match(p_user);
		if(result != -1)
		{
			if(result == 0)
			{
				printf("match error!");
				break;
			}
			else
			{
				printf("match success, the user id: %d%d, ", p_user->id[0], p_user->id[1]);
				for(int i = 0; i < sizeof(p_user->name) ; i++)
					printf("%c", p_user->name[i]);
				printf("\r\n");
				break;
			}
		}
		data_recieve_finished = 0;
	}
	data_recieve_finished = 0;
}

uint8_t HeadCompare(uint8_t data)
{
	static uint32_t index = 0;
	if (data == head_bytes[index])
	{
		if (index == 1)
		{
			index = 0;
			current_step++;
			return GET_LENGTH;
		}
		else
		{
			index++;
			current_step++;
			return GET_HEAD;
		}
	}
	else
	{
		index = 0;
		current_step = 0;
		return GET_HEAD;
	}
} 

uint8_t GetDataLength(uint8_t data, P_RECV_MSG p_recv_msg)
{
	static uint32_t index_3 = 0;
	if(current_step > 2)
	{
		p_recv_msg->data_length[index_3] = data;
		if (index_3 == 0)
		{
			index_3++;
			current_step++;
			return GET_LENGTH;
		}
		else if(index_3 == 1)
		{
			index_3 = 0;
			length = p_recv_msg->data_length[0]*256 + p_recv_msg->data_length[1]-2;
			current_step++;
			return GET_CMD;
		}
	}

	current_step++;
	return GET_LENGTH;

}

uint8_t VerifyDataCMD(uint8_t data, P_RECV_MSG p_recv_msg)
{
 	if (current_step > 4)
	{
		if (data == p_recv_msg->cmd)
		{
			current_step++;
			return GET_STATE;
		}
		else
		{
			current_step = 0;
			return GET_HEAD;
		}
 	}
	return -1;
}

uint8_t GetModuleState(uint8_t data, P_RECV_MSG p_recv_msg)
{
	if(current_step > 5)
	{
		p_recv_msg->state = data;

		current_step++;
		return GET_DATA;

	}
	return -1;
}

uint8_t DataStorage(uint8_t data, P_RECV_MSG p_recv_msg)//RECV_MSG *recv_msg
{
	static uint32_t index_2 = 0;
	if(length == 0)
		VerifyParity(data, p_recv_msg);
	if (current_step > 6)
	{
		p_recv_msg->data[index_2] = data;
		++index_2;
		if (index_2 >= length)
		{
			current_step++;
			index_2 = 0;
			return GET_PARITY;
		}
		else
		{
			current_step++;
			return GET_DATA;
		}
	}
	return -1;
}

uint8_t VerifyParity(uint8_t data, P_RECV_MSG p_recv_msg)
{
	uint8_t nParityCheck = 0;
	nParityCheck ^= p_recv_msg->data_length[0];
	nParityCheck ^= p_recv_msg->data_length[1];
	nParityCheck ^= p_recv_msg->cmd;
	nParityCheck ^= p_recv_msg->state;

	for (uint32_t i = 0; i < length; i++)
		nParityCheck ^= p_recv_msg->data[i];
	if (nParityCheck == data)
		data_recieve_finished = 1;
	current_step = 0;
	return GET_HEAD;

}

void UART_Compare(uint8_t data, P_RECV_MSG p_recv_msg)
{
	//RECV_MSG recv_msg = *p_recv_msg;
	static int state = 0;
	if (state == GET_HEAD)
		state = HeadCompare(data);
	else if (state == GET_LENGTH)
		state = GetDataLength(data, p_recv_msg);
	else if (state == GET_CMD)
		state = VerifyDataCMD(data, p_recv_msg);
	else if (state ==  GET_STATE)
		state = GetModuleState(data, p_recv_msg);
	else if (state == GET_DATA)
		state = DataStorage(data, p_recv_msg);
	else if (state ==  GET_PARITY)
		state = VerifyParity(data, p_recv_msg);	
	else
	{
		current_step = 0;
		state = GET_HEAD;
	}
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
	ring_buff[ring_buff_index] = g_fUART1_Byte;
	UART_Compare(ring_buff[ring_buff_index], &function);
	++ ring_buff_index;
	if(ring_buff_index >= 100)
	{
		ring_buff_index = 0;
	}
	if(ring_buff_index == 41)
	{
		ring_buff_index = 41;
	}
  /* USER CODE END USART1_IRQn 1 */
}

void Version_Send(uint8_t * data, uint32_t data_len)
{
	uint32_t temp_i;
	for(temp_i = 0; temp_i < data_len; ++ temp_i)
	{
		printf("%c", (char) data[temp_i]);
	}
	printf("\r\n");

}
void State_of_DeleteAll(void)
{
	if(function.state == 0x00)
		printf("Delete All Id success!");
	else
		printf("Delete All Id Fail!");
}

void State_of_DeleteID(uint8_t id)
{
	if(function.data_length[1] == 0x02 && function.cmd == 0x20)
	{
		if(function.state == 0x00)
			printf("Delete Id: %d success!", id);
		else
			printf("Delete Id: %d fail!", id);
	}
	else
		printf("error respond");
}
int State_of_Match(P_USER p_user)
{
	if(function.data_length[1] == 0x26)
	{
		if(function.state == 0x00)
		{
			p_user->id[0] = function.data[0];
			p_user->id[1] = function.data[1];
			for(uint8_t i = 0; i < 32; i++)
			{
				p_user->name[i] = function.data[i+2];
			}
			p_user->isAdm = function.data[33];
			return 1;
		}
		return 0;
	}
	else
		return -1;
}
int State_of_Enrollment(P_USER p_user, uint8_t dir, uint8_t enrollment_pattern)
{
	if(function.data_length[1] == 0x05)
		if(function.state == 0x00 && function.data[2] == dir)
		{
			if(enrollment_pattern == 1 || dir == 0x1F)
			{
				p_user->id[0] = function.data[0];
				p_user->id[1] = function.data[1];
			}
			return 1;
		}
		else
			return 0;
	else
		return -1;
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
  GetFirmwareVersion(huart1);
  */
 /*
  DeleteAll(huart1);
   DeleteID(huart1, 1);*/
  
 /*
  Match(huart1, &user);
   */
	
  user.isAdm = 0x00;
  char name[32] = "micheng";
  char str_dir[][5] = {"front", "up", "down", "left", "right"};
  memcpy(user.name, name, 32);
	for (int i = 0; i < 5 ; i++)
	{
		printf("pls look %s", str_dir[i]);
		int result = Enrollment(huart1, &user, i, 5);
		if(result == 0)
		{
			printf("enroll timeout!");
			break;
		}
		else if (result == 2)
		{
			printf("enrollment success, the user ");
			for (int a = 0; a< sizeof(user.name); a++)
				printf("%c", user.name[a]);
			printf("'s id: %d%d", user.id[0], user.id[1]);
			break;
		}		
	}
	/**/
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin : PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
uint8_t Check_UartIsOK(void)
{
	static uint16_t calc_T = 0;
	//如果flag不为1则表示没触发中断，没有数据接�?
	//同时也表示如果一直有数据发�?�过来则会一直让其处于等待数据接收完毕的状�??
	if(uart_get_flag)
	{
		uart_get_flag = 0;
		calc_T = 0xFFFF;
	}
	//中断触发后会进行等待
	if(calc_T)
	{
		-- calc_T;
		//等待完成即可打印数据
		if(0 == calc_T)
		{
			return 1;
		}
	}
	return 0;
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
