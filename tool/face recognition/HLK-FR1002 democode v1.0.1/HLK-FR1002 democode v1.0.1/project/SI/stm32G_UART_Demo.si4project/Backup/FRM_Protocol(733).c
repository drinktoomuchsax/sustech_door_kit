#ifndef __FRM_PROTOCOL_H__
#include "FRM_Protocol.h"



uint8_t ring_buff[100];
uint8_t uart_get_flag = 0;
uint8_t data_recieve_finished = 0;
uint8_t head_bytes[2] = {0xEF, 0xAA};
volatile int state = 0;
uint32_t length;
static uint32_t ring_buff_index = 0;
uint32_t current_step = 0;

uint8_t VerifyParity(uint8_t data, P_RECV_MSG p_recv_msg);

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


/************************************************************************
function name: Receive_Interruptiion
args:uint8_t g_fUART1_Byte, P_RECV_MSG p_function
************************************************************************/
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
		return VerifyParity(data, p_recv_msg);
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
	{
		for(int i = 0; i<sizeof(ring_buff); i++)
			ring_buff[i] = 0x00;
		ring_buff_index = -1;
		data_recieve_finished = 1;
	}
	current_step = 0;
	return GET_HEAD;

}

void UART_Compare(uint8_t data, P_RECV_MSG p_recv_msg)
{
	//RECV_MSG recv_msg = *p_recv_msg;

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

void Receive_Interruptiion(uint8_t g_fUART1_Byte, P_RECV_MSG p_function)
{
	if(ring_buff_index >= 100)
			ring_buff_index = 0;

	ring_buff[ring_buff_index] = g_fUART1_Byte;
	UART_Compare(ring_buff[ring_buff_index], p_function);
	ring_buff_index++;
	
}

/************************************************************************
function name: ML_API_PowerDown
args:P_RECV_MSG p_function
************************************************************************/

void State_of_PowerDown(P_RECV_MSG p_function)
{
	if(p_function->state == 0x00)
		printf("Module PowerDown!");
	else
		printf("Module PowerDown Fail!");
}

void ML_API_PowerDown(P_RECV_MSG p_function) 
{
	uint8_t pCmd1[6] = { 0xEF, 0xAA, 0xED, 0x00, 0x00, 0xED };
	p_function->cmd = 0xED;

	MSG_Send(pCmd1, 6);
	//-------------------------------------------------
	uint8_t t;
	uint8_t nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)	
		printf("PowerDown cmd sending with no respond!");
	else
		State_of_PowerDown(p_function);
	data_recieve_finished = 0;
}

/************************************************************************
function name: ML_API_ResetAndStandby
args:P_RECV_MSG p_function
************************************************************************/

void State_of_ResetAndStandby(P_RECV_MSG p_function)
{
	if(p_function->state == 0x00)
		printf("Reset module and standby");
	else
		printf("Reset Fail!");
}

void ML_API_ResetAndStandby(P_RECV_MSG p_function) 
{
	uint8_t pCmd1[6] = { 0xEF, 0xAA, 0x10, 0x00, 0x00, 0x10 };
	p_function->cmd = 0x10;

	MSG_Send(pCmd1, 6);
	//-------------------------------------------------
	uint8_t t;
	uint8_t nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)
		printf("ResetAndStandby cmd sending with no respond!");
	else
		State_of_PowerDown(p_function);
	
	data_recieve_finished = 0;
}

/************************************************************************
function name: ML_API_GetAllUserID
args:P_RECV_MSG p_function
************************************************************************/

uint8_t State_of_GetAllUserID(P_RECV_MSG p_function)
{
	if(p_function->state == 0x00)
		return p_function->data[0];
	else
		printf("Cannot get all users id!");
	return NULL;
}

uint8_t ML_API_GetAllUserID(P_RECV_MSG p_function) 
{
	uint8_t pCmd1[6] = { 0xEF, 0xAA, 0x24, 0x00, 0x00, 0x24 };
	uint8_t id_sum = -1;
	p_function->cmd = 0x24;

	MSG_Send(pCmd1, 6);
	//-------------------------------------------------
	uint8_t t;
	uint8_t nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)
		printf("GetAllUserID cmd sending with no respond!");
	else
		id_sum = State_of_GetAllUserID(p_function);
	
	data_recieve_finished = 0;
	return id_sum;
}


/************************************************************************
function name: ML_API_DeleteID
args:uint8_t uint8_t id, P_RECV_MSG p_function
************************************************************************/
void State_of_DeleteID(uint8_t id, P_RECV_MSG p_function)
{
	if(p_function->data_length[1] == 0x02 && p_function->cmd == 0x20)
	{
		if(p_function->state == 0x00)
			printf("Delete Id: %d success!", id);
		else
			printf("Delete Id: %d fail!", id);
	}
	else
		printf("error respond");
}

void ML_API_DeleteID(uint8_t id, P_RECV_MSG p_function)
{
	uint8_t pCmd1[8] = { 0xEF, 0xAA, 0x20, 0x00, 0x02 };
	p_function->cmd = 0x20;
	pCmd1[5] = (id&0xff00)>>8;//取高8位
	pCmd1[6] = id&0xff;//取低8位
	pCmd1[7] = GetParityCheck(pCmd1, sizeof(pCmd1));
	uint8_t nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	MSG_Send(pCmd1, 8);
	//-------------------------------------------------
	uint8_t t;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)
		printf("match cmd sending with no respond!");
	else
		State_of_DeleteID(id, p_function);
	
	data_recieve_finished = 0;
}

/************************************************************************
function name: ML_API_DeleteAll
args:P_RECV_MSG p_function
************************************************************************/
void State_of_DeleteAll(P_RECV_MSG p_function)
{
	if(p_function->state == 0x00)
		printf("Delete All Id success!");
	else
		printf("Delete All Id Fail!");
}

void ML_API_DeleteAll(P_RECV_MSG p_function) 
{
	uint8_t pCmd1[6] = { 0xEF, 0xAA, 0x21, 0x00, 0x00, 0x21 };
	p_function->cmd = 0x21;

	MSG_Send(pCmd1, 6);
	//-------------------------------------------------
	uint8_t t;
	uint8_t nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)
		printf("DeleteAll cmd sending with no respond!");
	else
		State_of_DeleteAll(p_function);
	
	data_recieve_finished = 0;
}

/************************************************************************
function name: ML_API_Match
args:P_USER p_user, P_RECV_MSG p_function
************************************************************************/
int State_of_Match(P_USER p_user, P_RECV_MSG p_function)
{
	if(p_function->data_length[1] == 0x26)
	{
		if(p_function->state == 0x00)
		{
			p_user->id[0] = p_function->data[0];
			p_user->id[1] = p_function->data[1];
			for(uint8_t i = 0; i < 32; i++)
			{
				p_user->name[i] = p_function->data[i+2];
			}
			p_user->isAdm = p_function->data[33];
			return 1;
		}
		return 0;
	}
	else
		return -1;
}

int ML_API_Match(P_USER p_user, P_RECV_MSG p_function)
{
	uint8_t pCmd1[] = { 0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x01, 0x11 };
	p_function->cmd = 0x12;

	MSG_Send(pCmd1, 8);
	
	int result;
	int nCycleTime = (0x03+1) * 1000 / NORM_CMD_TIME_OUT;
	int t = 0;
	while (1){
		for (; t < nCycleTime; ++t)
			{
				if(data_recieve_finished == 1)
					break;
				else
					Delay_ms(NORM_CMD_TIME_OUT);
			}

		if(t>=nCycleTime)
		{
			data_recieve_finished = 0;
			return -1;
		}

		result = State_of_Match(p_user, p_function);
		if(result != -1)
		{
			if(result == 0)
			{
				data_recieve_finished = 0;
				return 0;
			}
			else
			{
				data_recieve_finished = 0;
				return 1;
			}
		}
		data_recieve_finished = 0;
	}

}

/************************************************************************
function name: ML_API_GetFirmwareVersion
args:P_RECV_MSG p_function
************************************************************************/

void Print_Message(uint8_t * data, uint32_t data_len)
{
	uint32_t temp_i;
	for(temp_i = 0; temp_i < data_len; ++ temp_i)
	{
		printf("%c", (char) data[temp_i]);
	}
	printf("\r\n");

}

void ML_API_GetFirmwareVersion(P_RECV_MSG p_function)
{
	uint8_t pCmd1[8] = { 0xEF, 0xAA, 0x30, 0x00, 0x00, 0x30};
	p_function->cmd = 0x30;

	MSG_Send(pCmd1, 6);
	//-------------------------------------------------
	uint8_t t;
	int nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)
		printf("match cmd sending with no respond!");
	else
		Print_Message(p_function->data, p_function->data_length[0]*256+p_function->data_length[1]-2);
	
	data_recieve_finished = 0;
}

/************************************************************************
function name: ML_API_GetUUID
args:P_RECV_MSG p_function
************************************************************************/
void ML_API_GetUUID(P_RECV_MSG p_function)
{
	uint8_t pCmd1[8] = {0xEF, 0xAA, 0x93, 0x00, 0x00, 0x93};
	p_function->cmd = 0x93;
	
	MSG_Send(pCmd1, 6);
	//-------------------------------------------------
	uint8_t t;
	int nCycleTime = 1 * 1000 / NORM_CMD_TIME_OUT;
	for(t = 0; t < nCycleTime; ++t)
		if(data_recieve_finished)
			break;
		else
			Delay_ms(NORM_CMD_TIME_OUT);
		
	if(t>=nCycleTime)
		printf("match cmd sending with no respond!");
	else
		Print_Message(p_function->data, p_function->data_length[0]*256+p_function->data_length[1]-2);
	
	data_recieve_finished = 0;
}

/************************************************************************
function name: ML_API_Enrollment
args:P_RECV_MSG p_function, P_USER p_user, uint8_t dir, uint8_t enrollment_pattern
************************************************************************/
int State_of_Enrollment(P_RECV_MSG p_function, P_USER p_user, uint8_t dir, uint8_t enrollment_pattern)
{
	if(p_function->data_length[1] == 0x05)
		if(p_function->state == 0x00 && p_function->data[2] == dir)
		{
			if(enrollment_pattern == 1 || dir == 0x1F)
			{
				p_user->id[0] = p_function->data[0];
				p_user->id[1] = p_function->data[1];
			}
			return 1;
		}
		else
			return 0;
	else
		return -1;
}
   
int ML_API_Enrollment(P_RECV_MSG p_function, P_USER p_user, uint8_t dir, uint8_t enrollment_pattern)
{
	uint8_t pEnrollCmd[] = { 0xEF, 0xAA, 0x26, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0A,
		   0x00, 0x00, 0x00, 0x1E };
	p_function->cmd = 0x26;
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
	int nWaiteTime = 50;
	
	pEnrollCmd[41] = nTimeOut;
	pEnrollCmd[45] = GetParityCheck(pEnrollCmd, sizeof(pEnrollCmd));

	int nCycleTime = nTimeOut * 1000 / nWaiteTime;

	MSG_Send(pEnrollCmd, sizeof(pEnrollCmd));
	
	//----------------------
	int result;
	int t;
	int enroll_success = 0;
	int enrolling = 0;
	if (enrollment_pattern == 1)
	{
		while (1)
		{
			for (t = 0; t < nCycleTime; ++t)
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

			result = State_of_Enrollment(p_function, p_user, pAns[dir], enrollment_pattern);
			if(result != -1)
			{
				if(result == 0)
				{
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
			for (t = 0; t < nCycleTime; ++t)
			{
				if(data_recieve_finished)
					break;
				else
					Delay_ms(nWaiteTime);
			}
			printf("%d\n", t);
			if(t>=nCycleTime)
			{
				printf("enrollment cmd sending with no respond!");
				enrolling = 0;
				enroll_success = 0;
				break;
			}

			result = State_of_Enrollment(p_function, p_user, pAns[dir], enrollment_pattern);
			
			if(result != -1)
			{
				if(result == 0)
				{
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


#endif

