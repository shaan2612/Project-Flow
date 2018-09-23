#include "msp430.h"

//USCI_A0
//#define UART_TX_READY       (UCA0IFG & UCTXIFG)
//#define UART_RX_READY       (UCA0IFG & UCRXIFG)
//#define UART_TX_DONE        (UCA0IFG & UCTXCPTIFG)
//#define UART_RESET_TX_DONE  (UCA0IFG &= ~UCTXCPTIFG)
int i, d, k, b;



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog

    // Configure GPIO
    P3OUT &= ~(BIT0|BIT1);                       // Clear P2.0 output latch
    P3DIR |= BIT0|BIT1;                        //LED & Module
    P3OUT |= BIT0|BIT1;

    P4DIR |= (BIT0);                                 //5V enable
    P4OUT |= BIT0;

    P1OUT &= ~(BIT4 | BIT5 | BIT1 | BIT2);
    P1DIR |= BIT4 | BIT5 | BIT1 | BIT2;             //Bit 4 RFSD, Bit 5 Mode0, Bit 1 Buzz, Bit2 Sol
    P1OUT |= BIT5 ;                                   // Transmit Mode RF
    P1OUT &= ~BIT4;                                   // Switch off RF

    P2SEL1 |= BIT0 | BIT1;                    // USCI_A0 UART operation
    P2SEL0 &= ~(BIT0 | BIT1);


    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

      CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
      CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
      CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
      CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers

    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
    UCA0BR0 = 104;                             // 1000000/9600 UART config
    UCA0BR1 = 0x00;
    UCA0MCTLW |= 0x1100;
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    TA0CCTL0 = CCIE;                              // CCR0 interrupt enabled
    TA0CTL = TASSEL__ACLK | MC__UP| ID_3;// ACLK=VLO/8, upmode, clock = 10Khz/8 = 1.25Khz = .8ms
    __delay_cycles(1000000);
    P3OUT ^= BIT0|BIT1;
    TA0CCR0 = 1; //start timer
    __bis_SR_register(LPM2_bits | GIE);

}

// Delay
void delay_ms(unsigned int ms )
{
    unsigned int i;
    for (i = 0; i<= ms; i++)
       __delay_cycles(500); //Built-in function that suspends the execution for 500 cicles
}

//Buzzer
void beep()
{

    for (i=0;i<500;i++)
    {
        P1OUT |= BIT1;     //Set P1.1 Buzzer
        __delay_cycles(500); //Square wave
        P1OUT &= ~BIT1;    // reset Buzzer...
        __delay_cycles(500);
    }
    delay_ms(d);

}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

    if (k == 1){
     P3OUT &= ~BIT1;    // DEbug LED
     P1OUT &= ~BIT4;   //OFF RF
     TA0CCR0 = 1250-1; //1000ms
     k = 0;
    }
    else {
     P3OUT |= BIT1;    // DEbug LED
     P1OUT |= BIT4;   //ON RF
     TA0CCR0 = 80-1; //64ms
     k = 1;
    }
}

//UART SR
 #if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
 #pragma vector=USCI_A0_VECTOR
 __interrupt void USCI_A0_ISR(void)
 #elif defined(__GNUC__)
 void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
 #else
 #error Compiler not supported!
 #endif
 {
   switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
   {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:

        if (UCA0RXBUF == 'R') { // 'O' received?   ROLL BACK OVERFLOW
            P3OUT &= ~(BIT0|BIT1);        // LED off
            P1OUT &= ~(BIT2|BIT1);        // Sol & buzzer off
            TA0CCTL0 = CCIE;   // start timer
            TA0CCR0 = 40-1;     // start count with 32ms delay
            d = 0;
            break;
        }

        if (UCA0RXBUF == 'U') { // 'U' received?  OVERFLOW BYTE
            P1OUT |= (BIT2);        // Sol ON
            P3OUT |= (BIT0|BIT1);   // Turn Both LED ON
            TA0CCTL0 &= ~CCIE; //stop timer
            P1OUT |= BIT4;   //ON RF
            beep();
            d = 100;                 // Tone Delay
            break;
        }
        __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
       break;
     case USCI_UART_UCTXIFG: break;
     case USCI_UART_UCSTTIFG: break;
     case USCI_UART_UCTXCPTIFG: break;
   }

 }


