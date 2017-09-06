/*
 * Sound.h
 *
 *  Created on: Oct 5, 2016
 *      Author: aamark
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <stdint.h>
#include <XMC1300.h>
#include "CCU4.h"
#include "CCU8.h"

#define WATCHDOG_FREQ 16777215 // (2^24)-1 -> Systick is a 24 bit Timer

void PlaySound();
void SoundFinished();

#endif /* SOUND_H_ */
