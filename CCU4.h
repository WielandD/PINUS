/*
 * CCU4.h
 *
 *  Created on: Jun 25, 2016
 *      Author: Andreas Mark
 */

#ifndef CCU4_H_
#define CCU4_H_

#include <DAVE.h>
#include <xmc_ccu4.h>
#include <XMC1300.h>
#include "CCU8.h"
#include "DaisyCodes.h"

#define BlockCommutation_ISR IRQ_Hdlr_21
#define RampUp_ISR			 IRQ_Hdlr_22
//wir
#define CCU40_2_Capture_ISR	IRQ_Hdlr_23
//
//#define Daisy_WatchDog_ISR	 IRQ_Hdlr_23
//
#define MODULE_PTR      	CCU40
#define CAPTURE_SLICE_PTR       CCU40_CC41
#define CAPTURE_SLICE_NUMBER    (1U)
//wir
#define DUTY_DISARMED 16050
#define DUTY_ARM_MIN_VAL 16500
#define DUTY_ARM_MAX_VAL 29700

#define SLOPE ( 65535.0f/((float)(DUTY_ARM_MAX_VAL - DUTY_ARM_MIN_VAL)) )
#define OFFSET ( 65535.0f - SLOPE * ((float)(DUTY_ARM_MAX_VAL)) )
//

void InitCCU40_1Capture();
void InitCCU4();
void NextCommutationPattern();
void StartSlicesCCU4();
void StopSlicesCCU4();

#endif /* CCU4_H_ */
