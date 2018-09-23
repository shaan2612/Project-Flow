/*
 * timer.c
 *
 *  Created on: Mar 5, 2018
 *      Author: farshid
 */

#include <msp430.h>
#include "timer.h"
#include "adc.h"
#include "algorithm.h"


extern int adcval;

void timer_start(long count) {
    TA0CCTL0 = CCIE;                            // TACCR0 interrupt enabled
    TA0CCR0 = count;                            //TAxCCRn holds the data for the comparison
    TA0CTL = TASSEL__ACLK | MC__UP;             // ACLK, UP mode
}
void timer_cont(long count) {
    TA0CCR0 = count;                            //TAxCCRn holds the data for the comparison
}

void timer_stop(void) {
    TA0CCTL0 &= ~CCIE;
}


// Timer0_A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    //P4OUT |= BIT0;               //TUEN ON THE OSC
    //P2OUT ^= BIT3;                //TOGGLE LED
//    __delay_cycles(200);
    adcval = ADC12MEM0;
    __bic_SR_register_on_exit(LPM0_bits);

}

