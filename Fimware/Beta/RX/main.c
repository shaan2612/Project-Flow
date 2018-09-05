
#include "msp430.h"

//USCI_A0
#define UART_TX_READY       (UCA0IFG & UCTXIFG)
#define UART_RX_READY       (UCA0IFG & UCRXIFG)
#define UART_TX_DONE        (UCA0IFG & UCTXCPTIFG)
#define UART_RESET_TX_DONE  (UCA0IFG &= ~UCTXCPTIFG)
int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog

  // Configure GPIO
  P3OUT &= ~(BIT0|BIT1|BIT2);                           // Clear P2.0 output latch
   P3DIR |= BIT0|BIT1|BIT2;                        //LED & Module
   P3OUT |= (BIT0|BIT1|BIT2);


   P1OUT &= ~(BIT4 | BIT5 | BIT2);
   P1DIR |= (BIT4 | BIT5 | BIT2);                        //Bit 4 RFSD, Bit 5 Mode0
   P1OUT |= BIT5;
   P1OUT &= ~BIT4;

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
  UCA0CTLW0 &= ~UCSWRST;


  TA0CCTL0 = CCIE;                              // CCR0 interrupt enabled
  TA0CTL = TASSEL__SMCLK | MC__CONTINOUS | ID_3;// SMCLK/8, upmode

  TA1CCTL0 = CCIE;                          // TACCR0 interrupt enabled
  TA1CTL = TASSEL__SMCLK | MC__CONTINOUS | ID_3;                 // ACLK, continuous mode

  __delay_cycles(10000000);
       P3OUT ^= (BIT0|BIT1);
   __delay_cycles(1000000);

   TA0CCR0 =  9; // Start timer
    __bis_SR_register(LPM3_bits | GIE);     // Enter LPM0

}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  //  P3OUT ^= (BIT1);    // DEbug LED
    P1OUT |= BIT4;   //Switch on
    UCA0IE |= UCRXIE;   // Enable uart Int
    TA0CTL &= ~(MC_1); // Stop timer 1
    TA0CCR0 =  1999; // Start timer 2

}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
case USCI_NONE: break;
   case USCI_UART_UCRXIFG:
       if (UCA0RXBUF == 'U') // 'U' received?
           {
           P3OUT |= (BIT0|BIT1);   // LED
           P1OUT |= (BIT2);        // Sol
            }
       if (UCA0RXBUF == 'O') // 'O' received?
                  {
                  P3OUT &= ~(BIT0|BIT1);   // LED
                  P1OUT &= ~(BIT2);        // Sol
                   }
       __no_operation();
         break;
       case USCI_UART_UCTXIFG: break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
     }
   }


 ///Timer A1 interrupt service routine
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void)
{
//    P3OUT ^= (BIT1);  // DEbug LED
    P1OUT &= ~BIT4;    // Switch Off
    TA0CCR0 =  9999;   // Start Timer 1

}
