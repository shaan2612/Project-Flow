#include <msp430.h> 
#include "timer.h"
#include "adc.h"
#include "algorithm.h"


int adcval;     //adc value in each timer event
int calb=0;     //calibration done or not

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    // Configure GPIO
    P3OUT &= ~(BIT0|BIT1|BIT2);                     // Clear P2.0 output latch
    P3DIR |= BIT0|BIT1|BIT2;                        // LED & buzzer

    P3OUT |= (BIT0|BIT1);

    P4DIR |= (BIT0);                                 //osc enable
    P4OUT |= BIT0;

    P1SEL0 |= BIT0;                                     //configure P1.0 for ADC
    P1SEL1 |= BIT0;

    // Configure the wireless module
    P1OUT &= ~(BIT4 | BIT5);
    P1DIR |= BIT4 | BIT5;                        //Bit 4 RFSD, Bit 5 Mode0
    P1OUT |= BIT5;
    P1OUT |= ~BIT4;
    //MODE
//   P1DIR &= (~BIT4);    // Set P2.1 SEL as Input
//   P1IES |= (~BIT4);    // Falling Edge 1 -> 0
//   P1IFG &= (~BIT4); // Clear interrupt flag for P2.1
//   P1IE |= (BIT4); // Enable interrupt for P2.1

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
   //UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
   //UCA0IE |= UCTXIE;


   P3OUT &= ~(BIT0);
  __delay_cycles(100000000);
  //initialize various system modules
  buffer_init();
  timer_start(1000); //was 1000
  adc_init();
  adc_start();

   while (1) {
    __bis_SR_register(LPM4_bits | GIE);
      switch (calb) {
          case 1:
              monitor();
              break;
          case 0:
              calibration();
              break;
      }
   }
}
