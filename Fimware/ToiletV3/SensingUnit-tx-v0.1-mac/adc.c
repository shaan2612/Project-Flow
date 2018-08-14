/*
 * adc.c
 *
 *  Created on: Mar 5, 2018
 *      Author: farshid
 */

#include <msp430.h>
#include "adc.h"
#include "timer.h"


//volatile int voltage_init = 0;
//int counter = 1;                              //this counter is used to set the voltage the in beginning of the operation
//int test_adc = 0;
//int voltage;

void adc_init(void)
{
      ADC12CTL0 &= ~ADC12ENC;                   // disable adc12
      ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_0; // Turn on ADC12, extend sampling time
                                                // to avoid overflow of results
      ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2 + ADC12SSEL0_L + ADC12PDIV0;       // Use sampling timer, repeated sequence
      ADC12MCTL0 = ADC12INCH_0;                 // ref+=AVcc, channel = A0
      //ADC12IER0 |= ADC12IE0;
      //ADC12CTL0 |= ADC12ENC;                     // Enable ADC12

}
void adc_start(void)
{
    ADC12CTL0 |= ADC12ENC | ADC12SC;             // Enable conversions + Start convn - software trigger
}
void adc_stop(void)
{
    ADC12CTL0 &= ~ADC12ENC;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
  {
    case  0: break;                         // Vector  0:  No interrupt
    case  2: break;                         // Vector  2:  ADC12MEMx Overflow
    case  4: break;                         // Vector  4:  Conversion time overflow
    case  6: break;                         // Vector  6:  ADC12HI
    case  8: break;                         // Vector  8:  ADC12LO
    case 10: break;                         // Vector 10:  ADC12IN
    case 12:                                // Vector 12:  ADC12MEM0 Interrupt
        ADC12IFGR2 &= ~ADC12HIIFG;          // Clear interrupt flag
        P2OUT ^= BIT3;
        __bic_SR_register_on_exit(LPM3_bits); // Exit active CPU
        break;
    case 14: break;                         // Vector 14:  ADC12MEM1
    case 16: break;                         // Vector 16:  ADC12MEM2
    case 18: break;                         // Vector 18:  ADC12MEM3
    case 20: break;                         // Vector 20:  ADC12MEM4
    case 22: break;                         // Vector 22:  ADC12MEM5
    case 24: break;                         // Vector 24:  ADC12MEM6
    case 26: break;                         // Vector 26:  ADC12MEM7
    case 28: break;                         // Vector 28:  ADC12MEM8
    case 30: break;                         // Vector 30:  ADC12MEM9
    case 32: break;                         // Vector 32:  ADC12MEM10
    case 34: break;                         // Vector 34:  ADC12MEM11
    case 36: break;                         // Vector 36:  ADC12MEM12
    case 38: break;                         // Vector 38:  ADC12MEM13
    case 40: break;                         // Vector 40:  ADC12MEM14
    case 42: break;                         // Vector 42:  ADC12MEM15
    case 44: break;                         // Vector 44:  ADC12MEM16
    case 46: break;                         // Vector 46:  ADC12MEM17
    case 48: break;                         // Vector 48:  ADC12MEM18
    case 50: break;                         // Vector 50:  ADC12MEM19
    case 52: break;                         // Vector 52:  ADC12MEM20
    case 54: break;                         // Vector 54:  ADC12MEM21
    case 56: break;                         // Vector 56:  ADC12MEM22
    case 58: break;                         // Vector 58:  ADC12MEM23
    case 60: break;                         // Vector 60:  ADC12MEM24
    case 62: break;                         // Vector 62:  ADC12MEM25
    case 64: break;                         // Vector 64:  ADC12MEM26
    case 66: break;                         // Vector 66:  ADC12MEM27
    case 68: break;                         // Vector 68:  ADC12MEM28
    case 70: break;                         // Vector 70:  ADC12MEM29
    case 72: break;                         // Vector 72:  ADC12MEM30
    case 74: break;                         // Vector 74:  ADC12MEM31
    case 76: break;                         // Vector 76:  ADC12RDY
    default: break;
  }
}

