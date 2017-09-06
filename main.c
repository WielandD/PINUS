/*
 * main.c
 *
 *  Created on: 2016 Jun 24 13:04:06
 *  Author: aamark
 */
#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <probe_scope.h>
#include "CCU8.h"
#include "CCU4.h"
#include "ADC.h"
#include "DaisyChain.h"
#include "Sound.h"

/**
 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code. It is responsible for
 * invoking the APP initialization dispatcher routine - DAVE_Init() and hosting the place-holder for user application
 * code.
 */
extern uint16_t PWMPeriod;
extern uint16_t PWMCompare;
extern uint16_t PWMCompareStart;
extern uint16_t PWMCompareRef;
extern int8_t MotorState;
extern volatile uint8_t motorCMD;
extern volatile uint16_t param;
extern volatile int32_t duty;
uint8_t startMotor=0;
uint8_t motorRunning=0;
uint16_t speed=5;

int main(void)
{
	DAVE_STATUS_t status;
	status = DAVE_Init();           /* Initialization of DAVE APPs  */
	InitADC();
	InitCCU8();
	InitCCU4();
	InitCCU40_1Capture();
	//ProbeScope_Init(50000);
	PlaySound();
	if(status == DAVE_STATUS_FAILURE)
	{
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1U)
		{

		}
	}
	while(1U)
	{
		/*if (startMotor && !motorRunning)
		{
			SynchronousStartCCU8();
			StartSlicesCCU4();

			motorRunning = 1;
		}
		else if(!startMotor && motorRunning)
		{
			StopSlicesCCU8();
			StopSlicesCCU4();

			motorRunning=0;
		}

		if (MotorState > 2)
			PWMCompareRef=(PWMPeriod+1-PWMCompareStart)/100.0*speed+PWMCompareStart;*/

		//DaisyChain();
		//---------------------------------------------------------------------------------------------------------------------------------------
		// Replace CCU Timer by Systick Timer as Watchdog Timer
//		SysTick_Config(WATCHDOG_FREQ); /* Configure SysTick to generate an interrupt with a frequency of 1.9 Hz -> longest possible time between to Systick Interrupts*/
//		//---------------------------------------------------------------------------------------------------------------------------------------
//		for(volatile int test = 0;test < 320000;test++);
		switch (motorCMD)
		{
//			case START_MOTOR:
//				SynchronousStartCCU8();
//				StartSlicesCCU4();
//				break;
			case STOP_MOTOR:
//				param = 0.0f;
//				//Test
//				StopSlicesCCU4();
//				StopSlicesCCU8();
				break;
			case SET_REF_CURRENT:
//				param = SLOPE * duty + OFFSET;
//				if(param > 65535.0f) param = 65535.0f;
//				if(param < 0.0f) param = 0.0f;
//				//Test
//				PWMCompareRef=(PWMPeriod+1-PWMCompareStart)/65535.0f*param+PWMCompareStart;
				break;
		}


	}
}
