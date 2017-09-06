/*
 * DaisyChain.c
 *
 *  Created on: 02.05.2015
 *      Author: Andreas
 */

#include "DaisyChain.h"

uint8_t FifoRecBuffer[DAISY_BUFFER_SIZE] = {0};
uint8_t FifoTransBuffer[DAISY_BUFFER_SIZE+1] = {0};

uint8_t cmd;
uint16_t params;

extern uint16_t PWMPeriod;
extern uint16_t PWMCompare;
extern uint16_t PWMCompareStart;
extern uint16_t PWMCompareRef;
extern int8_t MotorState;

extern uint16_t CurrentSpeed;
extern uint8_t playingSound;

void DaisyChain(void)
{
	uint32_t data=0;
	uint16_t checksum=0;
	uint16_t checksum_rec=0;
	uint8_t i=0;
	uint8_t rec=0;

	//Read data from UART buffer
	//UART_Receive(&UART_DaisyChain, FifoRecBuffer, DAISY_BUFFER_SIZE);
	while(rec != DAISY_START_BYTE)
		rec=XMC_UART_CH_GetReceivedData(UART_DaisyChain.channel);

	if (playingSound)
		return;

	while (((UART_DaisyChain.channel->TRBSR & USIC_CH_TRBSR_RBFLVL_Msk)>>USIC_CH_TRBSR_RBFLVL_Pos)<DAISY_BUFFER_SIZE);

	for (uint8_t i=0; i<DAISY_BUFFER_SIZE; i++)
	{
		FifoRecBuffer[i]=XMC_UART_CH_GetReceivedData(UART_DaisyChain.channel);
		if (i<DAISY_BUFFER_SIZE-2)
			checksum+=(uint16_t)(255-FifoRecBuffer[i]);
	}

	checksum++;
	checksum_rec=(uint16_t)((FifoRecBuffer[DAISY_BUFFER_SIZE-2] << 8) | FifoRecBuffer[DAISY_BUFFER_SIZE-1]);

	if (checksum != checksum_rec)
		DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_DY_Error);
	else
	{
//		//Reset WatchDog Slice	//wir
//		CCU40_CC42->TCCLR |= ((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);// TCC = 1: Timer Clear

		//---------------------------------------------------------------------------------------------------------------------------------------
		// Replace CCU Timer by Systick Timer as Watchdog Timer
		SysTick_Config(WATCHDOG_FREQ); /* Configure SysTick to generate an interrupt with a frequency of 1.9 Hz -> longest possible time between to Systick Interrupts*/
		//---------------------------------------------------------------------------------------------------------------------------------------

		DIGITAL_IO_SetOutputLow(&DIGITAL_IO_DY_Error);

		cmd = FifoRecBuffer[0];
		params =  (FifoRecBuffer[1] << 8 | FifoRecBuffer[2]);

		switch (cmd)
		{
			case START_MOTOR:
				SynchronousStartCCU8();
				StartSlicesCCU4();
				break;
			case STOP_MOTOR:
				StopSlicesCCU4();
				StopSlicesCCU8();
				break;
			case SET_REF_CURRENT:
				if (params)
				{
					if (MotorState > 1)
					{
						//PWMCompare=(PWMPeriod+1-PWMCompareStart)/65535.0*params+PWMCompareStart;
						PWMCompareRef=(PWMPeriod+1-PWMCompareStart)/65535.0*params+PWMCompareStart;
					}
					else if (MotorState == 0)
					{
						SynchronousStartCCU8();
						StartSlicesCCU4();
					}
				}
				else
				{
					StopSlicesCCU4();
					StopSlicesCCU8();
				}
				break;
			case PLAY_SOUND:
				PlaySound();
				break;
		}

		for (uint8_t j=0; j<DAISY_BUFFER_SIZE+1; j++)
			FifoTransBuffer[j]=0;

		checksum=0;
		FifoTransBuffer[0] = DAISY_START_BYTE;

		for(i=DAISY_MESSAGE_LENGTH; i<DAISY_BUFFER_SIZE-2; i++)
		{
			FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]=FifoRecBuffer[i];
			checksum+=(uint16_t)(255-FifoRecBuffer[i]);
		}

		if (MotorState == 3)
			data = CurrentSpeed;
		else
			data=0;

		//Status-Code
		FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]=ROTOR_SPEED;
		checksum+=(uint16_t)(255-FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]);
		i++;
		//Data
		FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]=(uint8_t)(data >> 8);
		checksum+=(uint16_t)(255-FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]);
		i++;
		FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]=(uint8_t)data;
		checksum+=(uint16_t)(255-FifoTransBuffer[i-DAISY_MESSAGE_LENGTH+1]);

		checksum++;

		FifoTransBuffer[DAISY_BUFFER_SIZE-1] = (uint8_t)(checksum>>8);
		FifoTransBuffer[DAISY_BUFFER_SIZE] = (uint8_t)checksum;

		UART_Transmit(&UART_DaisyChain, FifoTransBuffer, DAISY_BUFFER_SIZE+1);
	}
}
