#ifndef __FRM_PROTOCOL_H__
#define __FRM_PROTOCOL_H__

#include "stm32g0xx_hal.h"
#include <stdio.h>
#include <string.h>

#define GET_HEAD 		  (0)
#define GET_LENGTH 		  (1)
#define GET_CMD 		  (2)
#define GET_STATE 		  (3)
#define GET_DATA 		  (4)
#define GET_PARITY 		  (5)
#define END_RECIEVE 	  (6)
#define MAX_SIZE 		  (64)
#define NORM_CMD_TIME_OUT (50)


typedef struct
{
	uint32_t isAdm;
	uint8_t name[32];
	uint8_t id[2];
}USER, *P_USER;

typedef struct
{
	uint8_t data_length[2];
	uint8_t cmd;
	uint8_t state;
	uint8_t data[MAX_SIZE];
	uint8_t parity;
}RECV_MSG, *P_RECV_MSG;

void Receive_Interruptiion(uint8_t g_fUART1_Byte, P_RECV_MSG p_function);
void ML_API_DeleteID(uint8_t id, P_RECV_MSG p_function);
void ML_API_DeleteAll(P_RECV_MSG p_function);
void ML_API_GetFirmwareVersion(P_RECV_MSG p_function);
void ML_API_Match(P_USER p_user, P_RECV_MSG p_function);
int ML_API_Enrollment(P_RECV_MSG p_function, P_USER p_user, uint8_t dir, uint8_t enrollment_pattern);
void ML_API_GetUUID(P_RECV_MSG p_function);
uint8_t ML_API_GetAllUserID(P_RECV_MSG p_function);
void ML_API_ResetAndStandby(P_RECV_MSG p_function);
void ML_API_PowerDown(P_RECV_MSG p_function);


#endif
