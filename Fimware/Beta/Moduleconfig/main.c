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
  P3OUT &= ~(BIT0|BIT1);                           // Clear P2.0 output latch
   P3DIR |= (BIT0|BIT1);                        //LED
   P3OUT |= (BIT0|BIT1);


   P1OUT &= ~(BIT4 | BIT5 | BIT2 | BIT1);
   P1DIR |= (BIT4 | BIT5 | BIT2);                        //Bit 4 RFSD, Bit 5 Mode0, Bit 1 Buzz, Bit2 Sol
   P1OUT |= BIT5;
   P1OUT |= BIT4;


}


