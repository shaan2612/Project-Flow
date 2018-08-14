/*
 * timer.h
 *
 *  Created on: Mar 5, 2018
 *      Author: farshid
 */

#ifndef TIMER_H_
#define TIMER_H_

void timer_start(long count);
void timer_stop(void);
void FRAMWrite(long bufferIndex, int adcVal);


#endif /* TIMER_H_ */
