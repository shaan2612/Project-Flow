#include <msp430.h> 
#include "timer.h"
#include "adc.h"
#include "algorithm.h"


int adcval;     //adc value in each timer event
int calb=0;     //calibration done or not
int STATE = 0;
const int OVERFLOW = 3;


/**
 * main.c
 */
int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    // Configure GPIO
    P3OUT &= ~(BIT0|BIT1);                     // Clear P2.0 output latch
    P3DIR |= BIT0|BIT1;                        // LED
    P3OUT |= BIT0|BIT1;

    P4DIR |= (BIT0);                                 //osc enable
    P4OUT |= BIT0;

    P1SEL0 |= BIT0;                                     //configure P1.0 for ADC
    P1SEL1 |= BIT0;

    // Configure the wireless module
    P1OUT &= ~(BIT4 | BIT5);
    P1DIR |= BIT4 | BIT5;                        //Bit 4 RFSD, Bit 5 Mode0
    P1OUT |= BIT5 | BIT4;                              // Transmit Mode RF, set power up
    P1OUT |= BIT4;                              //  RF - OFF

    P2SEL1 |= BIT0 | BIT1;                    // USCI_A0 UART operation
    P2SEL0 &= ~(BIT0 | BIT1);


   // Disable the GPIO power-on default high-impedance mode to activate
   // previously configured port settings
   PM5CTL0 &= ~LOCKLPM5;
   // Startup clock system with max DCO setting ~8MHz
   CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
   CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
   CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
   CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // set all dividers
   CSCTL0_H = 0;                             // Lock CS registers

   // Configure USCI_A0 for UART mode
   UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
   UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
   // Baud Rate calculation
   // 8000000/(16*9600) = 52.083
   // Fractional portion = 0.083
   // User's Guide Table 21-4: UCBRSx = 0x04
   // UCBRFx = int ( (52.083-52)*16) = 1
   UCA0BR0 = 52;                             // 8000000/16/9600
   UCA0BR1 = 0x00;
   UCA0MCTLW |= UCOS16 | UCBRF_1;
   UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
   UCA0IE &= ~UCTXIE;
   __delay_cycles(10000000);
   P3OUT ^= (BIT0|BIT1);                   // Start-Up Led OFF

   __delay_cycles(100000000);
   P1OUT &= ~BIT4;  // RF-OFF
  //initialize various system modules
  buffer_init();
  timer_start(1000); //was 1000
  adc_init();
  adc_start();

   while (1) {
        __bis_SR_register(LPM0_bits | GIE);
        switch (calb) {
          case 1:
              monitor();
              break;
          case 0:
              calibration();
              break;
        }
        if (STATE == OVERFLOW) {
            timer_cont(20);
        }
        else
            timer_cont(1000);
    }
}
