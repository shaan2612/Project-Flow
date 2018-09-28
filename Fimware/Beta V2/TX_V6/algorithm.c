/*
 * algorithm.c
 *
 *  Created on: Mar 5, 2018
 *      Author: farshid
 */



#include <msp430.h>
#include "timer.h"
#include <stdio.h>


//-----------------USCI_A0-----------------------------------------------

#define bufferSize  3000                                //size for calibration process was 3000
#define baseSize 450                                    //size for base voltage calculation

unsigned int calbcount = 0;                     //counter to go through buffer and calbBuffer
const int BASE_calb = 0;
const int ACTIVITY_calb = 1;
const int FLUSH_calb = 2;
int STATE_calb = 0;                             //different states of the calibration process
int flush_timer = 0;                            //to check if more than 1.5 seconds has passed for the flush event
extern int adcval;                              //ADC value in each timer event
extern int calb;
//double flush_value =0;                    //value to hold the average of the flush events
double flush_value_temp = 0;
//double flush_wind = 0;                    //value to hold the average duration of flush events
double flush_str = 0;                       //start of a flush
double flush_stp = 0;                       //stop of a flush
unsigned int justflush = 0;                     //to see if a new flush has been occurred or not
unsigned int newflush = 1;                      //to see if a new flush has been occurred or not
//unsigned int flush = 0;                           //number of flushes in calibration process
//------------------------------------------------------------------------------------------

//---------------------variables for monitoring--------------------------------------------------------
int max_flush_value = 0;                                //value to hold the max voltage for the flush
double base = 0;                                            //the voltage of the base level
double flush_threshold = 0  ;                               //threshold for flush value
//double base_threshold = 0;                                    //threshold for activity or not
//double overflow_maxtime = 0;                            //value to hold the time for an overflow
//int overflowTime = 0;                                 //overflow time window
double current_value = 0;                                   //history of the ADC values
extern unsigned int overflowhappen = 0;                 //check to see if an overflow has happened or not
extern int STATE;
const double current_value_thres = 0.1;
const double new_value_thres = 0.9;
const int BASE = 0;
const int ACTIVITY = 1;
const int DANGER = 2;
extern const int OVERFLOW;
unsigned short x;
char message[7];
int n_bytes;

//----------------------FRAM initialization----------------------------------------------------------
#pragma NOINIT(buffer)
int buffer[bufferSize];
#pragma NOINIT(calbBuffer)
int calbBuffer[bufferSize];

#pragma NOINIT(overflowTime)
int overflowTime;
#pragma NOINIT(flush_wind)
double flush_wind;
#pragma NOINIT(flush_value)
double flush_value;
#pragma NOINIT(base_threshold)
double base_threshold;
#pragma NOINIT (overflow_maxtime)
double overflow_maxtime;
#pragma NOINIT(flush)
unsigned int flush;

//debugging
int debug = 0;

void buffer_init(void) {
    unsigned int i;
    for(i=0;i<bufferSize; i++) {
        calbBuffer[i] =0;
    }
    for(i=0; i<bufferSize; i++) {
        buffer[i] = 0;
    }

    flush_wind = 0;
    flush_value = 0;
    base_threshold = 0;
    overflow_maxtime = 0;
    overflowTime=0;
    flush =0;

}

void calibration(void) {

    P3OUT ^= BIT0;
    buffer[calbcount] = adcval;
    if (max_flush_value < buffer[calbcount]) {
        max_flush_value = buffer[calbcount];
    }
    if (calbcount <baseSize) {
        base = base + buffer[calbcount];
        P3OUT |= BIT0;
    }

    calbcount = calbcount +1;

    //calculating the features
    if (calbcount == bufferSize) {
        timer_stop();
        base = base/(baseSize+1);
        flush_threshold = (max_flush_value+base)/2;
        base_threshold = base;
        current_value = base;


        for (calbcount =0; calbcount<bufferSize;calbcount++){
            current_value = current_value_thres*current_value + new_value_thres*buffer[calbcount];
            if (STATE_calb == BASE_calb) {
                calbBuffer[calbcount] = 0;
                if (current_value > base_threshold) {
                    STATE_calb = ACTIVITY_calb;
                    continue;
                }
            }
            if (STATE_calb == ACTIVITY_calb) {
                calbBuffer[calbcount] = 1;
                if (current_value > flush_threshold) {
                    STATE_calb = FLUSH_calb;
                    flush_timer = flush_timer +1;
                    continue;
                }
                if (current_value < base) {
                    STATE_calb = BASE_calb;
                    continue;
                }
            }
            if (STATE_calb == FLUSH_calb) {
                calbBuffer[calbcount] = 4;
                flush_timer = flush_timer +1;
                if (current_value < flush_threshold) {
                    STATE_calb = ACTIVITY_calb;
                    flush_timer = 0;
                    continue;
                }
                if (flush_timer > 15) {
                    calbBuffer[calbcount] = 2;
                }
            }
        }
        for (calbcount=0;calbcount<bufferSize;calbcount++) {

            if (calbBuffer[calbcount] == 2) {
                justflush = 1;
                if (newflush==1) {
                    flush = flush+1;
                    newflush = 0;
                    flush_str = 1.0 *calbcount;
                    flush_stp = 1.0 *calbcount;
                    flush_value_temp = buffer[calbcount];
                }
                else {
                    flush_stp = 1.0*calbcount;
                    if (buffer[calbcount] > flush_value_temp) {
                        flush_value_temp = buffer[calbcount];
                    }
                }
            }
            if (calbBuffer[calbcount] == 1) {
                newflush=1;
                if (justflush==0 ) {
                    continue;
                }
                else {
                    justflush = 0 ;
                    flush_wind = flush_wind + (flush_stp - flush_str);
                    flush_value = flush_value + flush_value_temp;
                }
            }
        }

        flush_wind = flush_wind/flush;  //it should be divided by flush
        flush_value = flush_value/flush; //it should be divided by flush
        flush_threshold = (flush_value + base)/2;
        base_threshold = base; //was 1.1*base
        current_value = base;

        if (flush_wind < 20) {
            overflow_maxtime = 2*flush_wind; //was 1.5*flush_wind
        }
        else if( 20 <= flush_wind < 30 ) {
            overflow_maxtime = 1.7*flush_wind; //was 1.5*flush_wind
        }
        else{
            overflow_maxtime = 1.4*flush_wind; //was 1.5*flush_wind
            }
        P3OUT = BIT0 |BIT1;
        __delay_cycles(10000000);
        P3OUT &= ~(BIT0 |BIT1);
        __delay_cycles(10000000);

//// Debug LEDS
//        while(flush == 0) {
//                   P3OUT |= BIT0 |BIT1;
//               }
//               if ((flush == 1) || (flush > 2)) {
//                          P3OUT |= BIT1;
//                          __delay_cycles(10000000);
//                          P3OUT &= ~BIT1;
//                          __delay_cycles(10000000);
//               }
//               else if (flush == 2) {
//                          P3OUT |= BIT0;
//                          __delay_cycles(10000000);
//                          P3OUT &= ~BIT0;
//                          __delay_cycles(10000000);
//               }
//
//               if ((flush_value < (base+125)) && (base < (adcval+60))) {
//                       P3OUT |= BIT0;
//                       __delay_cycles(10000000);
//                       P3OUT &= ~BIT0;
//                       __delay_cycles(10000000);
//                       }
//                       else { P3OUT |= BIT1;
//                       __delay_cycles(10000000);
//                       P3OUT &= ~BIT1;
//                       __delay_cycles(10000000);
//                       }

        timer_start(1000);
        calb=1;
    }
}



void monitor(void){
    int i;
    P1OUT &= ~(BIT4);
    current_value = (current_value_thres*current_value + new_value_thres*adcval);
    switch (STATE) {
        case 0:
            P1OUT &= ~(BIT4);
            P3OUT = 0;
            if (current_value > base_threshold) {
                STATE = 1;
                break;
            }
            break;
        case 1:
            P1OUT &= ~(BIT4);
            P3OUT = 0;
            if (current_value > flush_value) {
                STATE = 2;
                break;
            }
            if (current_value < base_threshold){
                STATE = 0;
                break;
            }
            break;
        case 2:
            P1OUT |= BIT4;
            P3OUT ^= BIT1;
            overflowTime = overflowTime +1;
            if (overflowTime > overflow_maxtime){
                STATE = 3;
                break;
            }
            if (current_value < flush_value) {
                STATE = 1;
                overflowTime = 0;
                break;
            }
            break;
        case 3:
            P1OUT |= BIT4;
            if (current_value < flush_value) {
                STATE = 2;
                for(i=0;i<20;i++) {
                    UCA0TXBUF = 'R';
                    __delay_cycles(100000); //delay to increase burst time
                   }
                overflowTime = 0;
                overflowhappen = 0;
                break;
            }
            overflowhappen = 1;
            P3OUT = BIT1;
            //send to the rx board
            UCA0TXBUF = 'U';                     // Buzz, Overflow
           __delay_cycles(1000);      // Just to prevent any lockup/junk/erroneous behavior
            break;
    }
}

