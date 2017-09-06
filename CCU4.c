/*
 * CCU4.c
 *
 *  Created on: Jun 25, 2016
 *      Author: Andreas Mark
 */

#include "CCU4.h"
#include "Sound.h"
//wir
#include "CCU8.h"
//wir

extern uint8_t CrossingDetected;

uint8_t ComPattern=1;
uint16_t ComStartPeriod=9999;
uint16_t ComDelta=50;
//uint16_t ComEndPeriod=1999;
uint16_t ComEndPeriod=9999;
uint16_t ADCGatingPeriod=1280;
int8_t MotorState=0;
//
float param = 0.0f;
volatile uint8_t motorCMD = IDLE;

extern uint16_t PWMPeriod;
extern uint16_t PWMCompare;
extern uint16_t PWMCompareStart;
extern uint16_t PWMCompareRef;
extern int8_t MotorState;

//wir

uint32_t SOUND[]=
{
	14440,
	14440,
	0,
	14440,
	10810,
	14440,
	9626,
	9626,
	9626,
	0,
	0,
	0,
	0,
	0,
	9626,
	9626,
	9626

};


extern uint8_t playingSound;

//
//XMC Capture/Compare Unit 4 (CCU4) Configuration for PWM input:
/* Capture Slice configuration */
XMC_CCU4_SLICE_CAPTURE_CONFIG_t capture_config =
{
   .fifo_enable	= false,
   	/* Clear only when timer value has been captured in C1V and C0V */
   .timer_clear_mode = XMC_CCU4_SLICE_TIMER_CLEAR_MODE_CAP_LOW,
   .same_event = false,
   .ignore_full_flag = true,
   .prescaler_mode = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
   .prescaler_initval = (uint32_t) XMC_CCU4_SLICE_PRESCALER_4,
   .float_limit = (uint32_t) 0,
   .timer_concatenation	= (uint32_t) 0
};
//wir

//
XMC_CCU4_SLICE_EVENT_CONFIG_t capture_event0_config =  	//off time capture
{
  .mapped_input	= XMC_CCU4_SLICE_INPUT_C,		//CAPTURE on P0.1
  .edge 		= XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
  .level		= XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
  .duration 	= XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
};
//wir

//
XMC_CCU4_SLICE_EVENT_CONFIG_t capture_event1_config =	//on time capture
{
  .mapped_input	= XMC_CCU4_SLICE_INPUT_C,		//CAPTURE on P0.1
  .edge 		= XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE,
  .level 		= XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
  .duration 	= XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
};
//wir

//Project Variables Definition:
volatile int32_t duty;
volatile int32_t period;

//
/* Interrupt handler - at event 1 to read the captured value for the duty cycle and period of the input signal */
void CCU40_2_Capture_ISR(void)
{
	if (playingSound)
		return;

	/* Clear pending interrupt */
	XMC_CCU4_SLICE_ClearEvent(CAPTURE_SLICE_PTR, XMC_CCU4_SLICE_IRQ_ID_EVENT1);
	uint32_t capturedPeriod = XMC_CCU4_SLICE_GetCaptureRegisterValue(CAPTURE_SLICE_PTR,1U);
	uint32_t capturedDuty = XMC_CCU4_SLICE_GetCaptureRegisterValue(CAPTURE_SLICE_PTR,3U);
	/* Captured value for the period and duty cycle of the signal */
	period = capturedPeriod & CCU4_CC4_CV_CAPTV_Msk;
	duty = capturedDuty & CCU4_CC4_CV_CAPTV_Msk;
	if(period >= duty)
	{
		if(duty >= DUTY_ARM_MIN_VAL)
		{
			if(MotorState == 0)
			{
				motorCMD = START_MOTOR;
				//Test
				SynchronousStartCCU8();
				StartSlicesCCU4();
				/* Disable IRQ */
				//NVIC_DisableIRQ(CCU40_2_IRQn);
			}else if(MotorState > 1)
			{
				motorCMD = SET_REF_CURRENT;
				param = SLOPE * duty + OFFSET;
				if(param > 65535.0f) param = 65535.0f;
				if(param < 0.0f) param = 0.0f;
				//Test
				PWMCompareRef=(PWMPeriod+1-PWMCompareStart)/65535.0f*param+PWMCompareStart;
			}
			//---------------------------------------------------------------------------------------------------------------------------------------
			// Replace CCU Timer by Systick Timer as Watchdog Timer
			SysTick_Config(WATCHDOG_FREQ); /* Configure SysTick to generate an interrupt with a frequency of 1.9 Hz -> longest possible time between two Systick Interrupts*/
			//---------------------------------------------------------------------------------------------------------------------------------------
		}else
		{
				motorCMD = STOP_MOTOR;
				param = 0;
				//Test
				StopSlicesCCU4();
				StopSlicesCCU8();
		}
	}else
	{
				param = 0;
				//Test
				StopSlicesCCU4();
				StopSlicesCCU8();
	}
}
//wir

void InitCCU40_1Capture()
{
		//	•	Configure Slice(s) Functions, Interrupts and Start-up
		/* Initialize the Slice */
		XMC_CCU4_SLICE_CaptureInit(CAPTURE_SLICE_PTR, &capture_config);
		/* Enable shadow transfer for PWM and Capture Slices */
		XMC_CCU4_EnableShadowTransfer(MODULE_PTR,(uint32_t)(XMC_CCU4_SHADOW_TRANSFER_SLICE_1));
		/* Configure and enable events */
		XMC_CCU4_SLICE_Capture0Config(CAPTURE_SLICE_PTR, XMC_CCU4_SLICE_EVENT_0);
		XMC_CCU4_SLICE_Capture1Config(CAPTURE_SLICE_PTR, XMC_CCU4_SLICE_EVENT_1);
		XMC_CCU4_SLICE_ConfigureEvent(CAPTURE_SLICE_PTR, \
				XMC_CCU4_SLICE_EVENT_0, &capture_event0_config);
		XMC_CCU4_SLICE_ConfigureEvent(CAPTURE_SLICE_PTR, \
				XMC_CCU4_SLICE_EVENT_1, &capture_event1_config);
		XMC_CCU4_SLICE_EnableEvent(CAPTURE_SLICE_PTR, XMC_CCU4_SLICE_IRQ_ID_EVENT1);
		/* Connect capture on event 1 to SR2 */
		XMC_CCU4_SLICE_SetInterruptNode(CAPTURE_SLICE_PTR, \
				XMC_CCU4_SLICE_IRQ_ID_EVENT1, XMC_CCU4_SLICE_SR_ID_2);
		/* Configure NVIC */
		/* Set priority */
		NVIC_SetPriority(CCU40_2_IRQn, 3U);
		/* Enable IRQ */
		NVIC_EnableIRQ(CCU40_2_IRQn);
		/* Get the slice out of idle mode */
		XMC_CCU4_EnableClock(MODULE_PTR, CAPTURE_SLICE_NUMBER);
		//	Start Timer Running
		XMC_CCU4_SLICE_StartTimer(CAPTURE_SLICE_PTR);
}


void InitCCU4()
{
	//Make protected bitfields available for modification
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk);
									// MODE = 0:		Bit protection scheme disable
						  	  	  	// PROTS = 0:		Software is able to wirte to all protected bits
						  	  	  	// PASS = 0x18: 	Enable writing of bit field MODE

	//Loop until the lock is removed
	while(((SCU_GENERAL->PASSWD) & SCU_GENERAL_PASSWD_PROTS_Msk));

	SCU_CLK->CGATCLR0	|= ((1 << SCU_CLK_CGATCLR0_CCU40_Pos) & SCU_CLK_CGATCLR0_CCU40_Msk);
						   	   	   //CCU40 = 1:		disable gating

	//Wait voltage supply stabilization
	while ((SCU_CLK->CLKCR) & SCU_CLK_CLKCR_VDDC2LOW_Msk);

	//disable protected bits
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk) |
									// PASS = 0x18: 	Enable writing of bit field MODE
						  ((0x03 << SCU_GENERAL_PASSWD_MODE_Pos) & SCU_GENERAL_PASSWD_MODE_Msk);
									// MODE = 3:		Bit protection scheme enabled

	//Global IDLE clear
	CCU40->GIDLC	=  ((1 << CCU4_GIDLC_CS0I_Pos) & CCU4_GIDLC_CS0I_Msk) |
									// GIDLC = 1: Remove IDLE mode from slice 0
					   ((1 << CCU4_GIDLC_CS1I_Pos) & CCU4_GIDLC_CS1I_Msk) |
					   	   	   	    // GIDLC = 1: Remove IDLE mode from slice 1
					   ((1 << CCU4_GIDLC_CS2I_Pos) & CCU4_GIDLC_CS2I_Msk) |
					   				// GIDLC = 1: Remove IDLE mode from slice 2
					   ((1 << CCU4_GIDLC_CS3I_Pos) & CCU4_GIDLC_CS3I_Msk) |
					   				// GIDLC = 1: Remove IDLE mode from slice 3
					   ((1 << CCU4_GIDLC_SPRB_Pos) & CCU4_GIDLC_SPRB_Msk);
									// SPRB = 1: Set run bit of prescaler

	CCU40_CC40->TC |= ((1 << CCU4_CC4_TC_CLST_Pos) & CCU4_CC4_TC_CLST_Msk);
									//CLST = 1: Shadow transfer on clear
	CCU40_CC43->TC |= ((1 << CCU4_CC4_TC_TSSM_Pos) & CCU4_CC4_TC_TSSM_Msk);
									//TSSM = 1: Timer Single Shot Mode

	//Prescaler Configuration
	CCU40_CC40->PSC |= ((5 << CCU4_CC4_PSC_PSIV_Pos) & CCU4_CC4_PSC_PSIV_Msk);
									//PSIV = 5: Prescaler initial value is fccu4/32 see page 810 in datasheet
	CCU40_CC42->PSC |= ((5 << CCU4_CC4_PSC_PSIV_Pos) & CCU4_CC4_PSC_PSIV_Msk);//wir tauschen 1 mit 2
									//PSIV = 5: Prescaler initial value is fccu4/32 see page 810 in datasheet
//	CCU40_CC42->PSC |= ((10 << CCU4_CC4_PSC_PSIV_Pos) & CCU4_CC4_PSC_PSIV_Msk); //wir
//									//PSIV = 10: Prescaler initial value is fccu4/1024 see page 810 in datasheet

	//Period and Compare
	CCU40_CC40->PRS = ComStartPeriod;
	//CCU40_CC40->CRS = ComEndPeriod-ComDelta;
	CCU40_CC40->CRS = 2000;

	CCU40_CC42->PRS = ComStartPeriod;//wir tauschen 1 mit 2
//	CCU40_CC42->PRS = 0xffff; //wir auskommentiert
	CCU40_CC43->PRS = ADCGatingPeriod;


	//Global channel set
	CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU4_GCSS_S1SE_Pos) & CCU4_GCSS_S1SE_Msk) |
									// S1SE: Slice 1 shadow transfer set enable
					   ((1 << CCU4_GCSS_S2SE_Pos) & CCU4_GCSS_S2SE_Msk) |
									// S1SE: Slice 2 shadow transfer set enable
					   ((1 << CCU4_GCSS_S3SE_Pos) & CCU4_GCSS_S3SE_Msk);
					   				// S1SE: Slice 3 shadow transfer set enable

	//Interrupt Configuration
	CCU40_CC40->SRS		=  ((0 << CCU4_CC4_SRS_CMSR_Pos) & CCU4_CC4_SRS_CMSR_Msk);
									// CMSR = 0: Forward compare match up to CC4ySR0
	CCU40_CC42->SRS		=  ((1 << CCU4_CC4_SRS_POSR_Pos) & CCU4_CC4_SRS_POSR_Msk);//wir tauschen 1 mit 2
									// POSR = 1: Forward period match to CC4ySR1
//	CCU40_CC42->SRS		=  ((2 << CCU4_CC4_SRS_POSR_Pos) & CCU4_CC4_SRS_POSR_Msk); //wir
//									// POSR = 2: Forward period match to CC4ySR2

	//Interrupt Enable Control
	CCU40_CC40->INTE	=  ((1 << CCU4_CC4_INTE_CMUE_Pos) & CCU4_CC4_INTE_CMUE_Msk);
									// CMUE = 1: Compare Match while counting up enabled
	CCU40_CC42->INTE	=  ((1 << CCU4_CC4_INTE_PME_Pos) & CCU4_CC4_INTE_PME_Msk);
									// PME = 1: Period/One-Match enable
//	CCU40_CC42->INTE	=  ((1 << CCU4_CC4_INTE_PME_Pos) & CCU4_CC4_INTE_PME_Msk); //wir
//									// PME = 1: Period/One-Match enable

	NVIC_SetPriority(CCU40_0_IRQn,0);
	NVIC_EnableIRQ(CCU40_0_IRQn);
	NVIC_SetPriority(CCU40_1_IRQn,3);
	//NVIC_EnableIRQ(CCU40_1_IRQn);
    NVIC_SetPriority(CCU40_2_IRQn,3); //wir
	NVIC_EnableIRQ(CCU40_2_IRQn);
	//Start DaisyChain Watchdog
//	CCU40_CC42->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk); //wir
//									// TRBS = 1: Timer Run Bit set
}

void BlockCommutation_ISR()
{
	if (CCU40_CC40->INTS & CCU4_CC4_INTS_CMUS_Msk)
	{
		ComPattern++;

		if (ComPattern > 6)
			ComPattern=1;

		SetCommutationPattern(ComPattern);

		CCU40_CC43->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
										// TRBS = 1: Timer Run Bit set

		CrossingDetected=0;

		CCU40_CC40->SWR |= ((1 << CCU4_CC4_SWR_RCMU_Pos) & CCU4_CC4_SWR_RCMU_Msk);
								//RCMU=1:	Compare Match while counting up clear
	}
}

void RampUp_ISR()
{
	if (CCU40_CC42->INTS & CCU4_CC4_INTS_PMUS_Msk)
	{
		if (CCU40_CC40->PR > ComEndPeriod)
		{
			CCU40_CC40->PRS = CCU40_CC40->PR - ComDelta;

			CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk);
									// S0SE: Slice 0 shadow transfer set enable

			while (CCU40->GCST & CCU4_GCST_S0SS_Msk);
		}
		else
		{
			MotorState=2;
			NVIC_DisableIRQ((IRQn_Type)22);
			/* Enable IRQ */
			//NVIC_EnableIRQ(CCU40_2_IRQn);
		}
		CCU40_CC42->SWR |= ((1 << CCU4_CC4_SWR_RPM_Pos) & CCU4_CC4_SWR_RPM_Msk);
									//RPM=1:	Period/One Match while counting up clear
	}
}

//void Daisy_WatchDog_ISR()
//{
////	static uint8_t cnt=1;
////
////	if (CCU40_CC42->INTS & CCU4_CC4_INTS_PMUS_Msk)
////	{
////		if (playingSound)
////		{
////			if (cnt < (sizeof(SOUND) >> 2))
////			{
////				PlayFrequency(SOUND[cnt]);
////				cnt++;
////			}
////			else
////				SoundFinished();
////		}
////		else
////		{
////			DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_DY_Error);
////
////			StopSlicesCCU4();
////			StopSlicesCCU8();
////		}
////
////		CCU40_CC42->SWR |= ((1 << CCU4_CC4_SWR_RPM_Pos) & CCU4_CC4_SWR_RPM_Msk);
////								//RPM=1:	Period/One Match while counting up clear
////	}
//}

void SysTick_Handler(void)  {                               /* SysTick interrupt Handler.*/
	static uint8_t cnt=1;

//	if (CCU40_CC42->INTS & CCU4_CC4_INTS_PMUS_Msk) //wir
//	{
		if (playingSound)
		{
			if (cnt < (sizeof(SOUND) >> 2))
			{
				PlayFrequency(SOUND[cnt]);
				cnt++;
			}
			else
				SoundFinished();
		}
		else
		{
			DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_DY_Error);
			StopSlicesCCU4();
			StopSlicesCCU8();
		}
//		CCU40_CC42->SWR |= ((1 << CCU4_CC4_SWR_RPM_Pos) & CCU4_CC4_SWR_RPM_Msk); //wir
//								//RPM=1:	Period/One Match while counting up clear
//	}
}


void StartSlicesCCU4()
{
	CCU40_CC40->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
									// TRBS = 1: Timer Run Bit set
	CCU40_CC42->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
									// TRBS = 1: Timer Run Bit set
	MotorState=1;
}

void StopSlicesCCU4()
{
	CCU40_CC40->TCCLR	=	((1 << CCU4_CC4_TCCLR_TRBC_Pos) & CCU4_CC4_TCCLR_TRBC_Msk) |
										// TRBC=1: Timer Run Bit Clear
							((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
										// TCC=1: Timer Clear
	CCU40_CC42->TCCLR	=	((1 << CCU4_CC4_TCCLR_TRBC_Pos) & CCU4_CC4_TCCLR_TRBC_Msk) |
										// TRBC=1: Timer Run Bit Clear
							((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
										// TCC=1: Timer Clear

	//NVIC_EnableIRQ((IRQn_Type)22);

	//Period and Compare
	CCU40_CC40->PRS = ComStartPeriod;
	//CCU40_CC40->CRS = ComEndPeriod-ComDelta;
	CCU40_CC40->CRS = 2000;

	CCU40_CC42->PRS = ComStartPeriod;

	//Global channel set
	CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU4_GCSS_S1SE_Pos) & CCU4_GCSS_S1SE_Msk);
									// S1SE: Slice 1 shadow transfer set enable
}
