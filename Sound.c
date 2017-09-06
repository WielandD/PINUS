/*
 * Sound.c
 *
 *  Created on: Oct 5, 2016
 *      Author: aamark
 */

#include "Sound.h"

uint8_t playingSound=0;
extern uint16_t PWMPeriod;
extern uint16_t PWMCompare;
extern uint32_t SOUND[];


void PlaySound()
{
//	NVIC_DisableIRQ(CCU40_2_IRQn); //wir

	StopSlicesCCU4();
	StopSlicesCCU8();
	NVIC_DisableIRQ(CCU40_0_IRQn);

	//Configure CCU4 Watchdog at 4Hz -> wir
	//CCU40_CC42->PRS =15624; 		//0.25s
	//CCU40->GCSS = ((1 << CCU4_GCSS_S2SE_Pos) & CCU4_GCSS_S2SE_Msk);

	CCU80_CC80->PSC = 3;
	CCU80_CC81->PSC = 3;
	CCU80_CC82->PSC = 3;
	CCU80_CC83->PSC = 3;

	playingSound=1;
	//NVIC_EnableIRQ(CCU40_2_IRQn); //Enable CCU Watchdog ISR -> wir

	//---------------------------------------------------------------------------------------------------------------------------------------
	// Replace CCU Timer by Systick Timer as Watchdog Timer
	SysTick_Config(SystemCoreClock / 4); /* Configure SysTick to generate an interrupt with a frequency of 4 Hz, 32MHz / 4Hz = 8 * 10^6 Ticks*/
	//---------------------------------------------------------------------------------------------------------------------------------------

	PlayFrequency(SOUND[0]);
	SynchronousStartCCU8();
}

void SoundFinished()
{
	StopSlicesCCU8();
	StopSlicesCCU4();

	playingSound=0;

	CCU80_CC80->PSC = 0;
	CCU80_CC81->PSC = 0;
	CCU80_CC82->PSC = 0;
	CCU80_CC83->PSC = 0;

//	CCU40_CC42->PRS = 0xFFFF;  -> wir
//	CCU40->GCSS = ((1 << CCU4_GCSS_S2SE_Pos) & CCU4_GCSS_S2SE_Msk);

	//---------------------------------------------------------------------------------------------------------------------------------------
	// Replace CCU Timer by Systick Timer as Watchdog Timer
	SysTick_Config(WATCHDOG_FREQ); /* Configure SysTick to generate an interrupt with a frequency of 1.9 Hz -> longest possible time between to Systick Interrupts*/
	//---------------------------------------------------------------------------------------------------------------------------------------

	CCU80_CC80->PRS = CCU80_CC81->PRS = CCU80_CC82->PRS = CCU80_CC83->PRS = PWMPeriod;
	CCU80_CC80->CR1S = PWMCompare;
	CCU80_CC80->CR2S = PWMCompare;
	CCU80_CC81->CR1S = 0;
	CCU80_CC81->CR2S = 0;
	CCU80_CC82->CR1S = 0;
	CCU80_CC82->CR2S = PWMPeriod+1;
	CCU80_CC83->CR1S = PWMCompare >> 2;

	//Global channel set
	CCU80->GCSS 	=  ((1 << CCU8_GCSS_S0SE_Pos) & CCU8_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU8_GCSS_S1SE_Pos) & CCU8_GCSS_S1SE_Msk) |
									// S1SE: Slice 1 shadow transfer set enable
					   ((1 << CCU8_GCSS_S2SE_Pos) & CCU8_GCSS_S2SE_Msk) |
									// S2SE: Slice 2 shadow transfer set enable
					   ((1 << CCU8_GCSS_S3SE_Pos) & CCU8_GCSS_S3SE_Msk);
									// S3SE: Slice 3 shadow transfer set enable

	NVIC_EnableIRQ(CCU40_0_IRQn);
}

