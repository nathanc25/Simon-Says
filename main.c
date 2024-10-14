////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Lab 2
//** NAMES: Kaylee Xie, Nathan Chen
//** RIN: 662071485, 662069036
//** Section: 3
//** Side: A
//** Seat number: 20
////////////////////////////////////////////////////////////////////////



#include "engr2350_msp432.h"
#include <stdlib.h>


void GPIOInit();
void Timer_Init();

uint8_t bumpers();
void Timer_ISR();

uint8_t LEDOnCar = 0;

Timer_A_UpModeConfig config;
uint8_t numOverflows;
uint8_t gamestart = 1;
uint8_t timercount = 0;
uint8_t patternstart = 0;
uint8_t patterncomplete = 0;

int main(void) {    /** Main Function ****/
    SysInit(); // Basic car initialization
    GPIOInit();
    Timer_Init();

    //Print game instructions
    printf("\r\n\n"
           "=====================\r\n"
           "Let's play Simon Says\r\n"
           "=====================\r\n\n\n");

    printf("--------------------------------------------------------------------------------\r\n"
           "Instructions:\r\n\n"
           "An LED in the center of the car will light up with different colors in a random\r\n"
           "order. Press the bumper that corresponds to the correct color to move on to the\r\n"
           "next round. There will be 10 rounds in total. With each round, the sequence of\r\n"
           "colors will increase by one.\r\n\n"
           "If you win a round, then 1 will be added to your total score. However, if you lose\r\n"
           "a round, then you will have to start over from round 1 with a score of 0.\r\n"
           "You will also lose if you take more than 3 seconds to enter each successive bumper\r\n"
           "press when answering.\r\n"
           "You can win the game by passing all 10 rounds, getting a total score of 10.\r\n\n"
           "You may test which bumpers trigger which color at this time. When you are ready to\r\n"
           "start, press the push button on the circuit.\r\n"
           "--------------------------------------------------------------------------------\r\n");

    uint8_t sequence[10];
    uint8_t i;
    for (i=0; i<10; i++) {
        sequence[i] = rand()%6; // generates random nums between 0 and 5 (6 colors)
        printf("%u",sequence[i]);
    }

    uint8_t round = 1;



    while (1) {
        // while game has not started
        if (gamestart) {
            uint8_t PB1 = GPIO_getInputPinValue( GPIO_PORT_P5, GPIO_PIN6 );
            // if button is pressed, start game
            if (PB1) {
               printf("button pressed");
               gamestart = 0;
               timercount = 0;
               round = 1;

               while(timercount <= 20);
           }
            else {
               // check if bumper is pressed
               uint8_t bumper = bumpers();
           }
       }

       if (patterncomplete == 0 && gamestart == 0) {


            printf("\r\nRound %u\r\n", round);

            for (i = 0; i < round; i++) {

                if (sequence[i] == 0) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                }
                else if (sequence[i] == 1) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                }
                else if (sequence[i] == 2) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                }
                else if (sequence[i] == 3) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                }
                else if (sequence[i] == 4) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                }
                else if (sequence[i] == 5) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                }

                // wait .5 sec
                timercount = 0;
                while (timercount <= 10);

                // turn off all leds
                GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
            }

            // check input
            uint8_t correct = 1;
            for (i = 0; i < round; i++) {
                timercount = 0;
                while (timercount <= 60) {  // wait 3 secs
                    uint8_t bumper = bumpers();
                    if (bumper != 6 && bumper != sequence[i]) {    //wrong input
                        printf("bp %u", bumper);
                        printf("s %u", sequence[i]);
                        correct = 0;
                        break;
                    }
                    if (bumper == sequence[i]) {
                        correct = 1;
                        bumper = 6;
                        break;
                    }
                    if (timercount == 60) {
                        correct = 0;
                    }
                }

                if (!correct) {  //longer than 3 secs - incorrect
                    break;
                }
            }


            if (correct == 0) {
                printf("\nYou lost! You took longer than 3 seconds or entered the wrong sequence.\n");
                patterncomplete = 1;

                //flashes led red until PB pressed
                printf("Score: %u", round);
                GPIO_setOutputLowOnPin( GPIO_PORT_P6 , GPIO_PIN0 );
                GPIO_setOutputHighOnPin( GPIO_PORT_P6 , GPIO_PIN1 );
                while (GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6) == 0) {
                    timercount = 0;
                    while (timercount <= 5);
                    GPIO_toggleOutputOnPin(GPIO_PORT_P6, GPIO_PIN1);  // Toggle red LED
                }

                //restarts game
                GPIO_setOutputLowOnPin( GPIO_PORT_P6 , GPIO_PIN1 );
                gamestart = 1;
                round = 1;
                patterncomplete = 0;

            }
            else {
                //correct sequence -> move to next round
                round++;
                if (round > 10) {
                    printf("\nCongratulations, you won!\n");
                    patterncomplete = 1;

                    // flash green until PB pressed
                    GPIO_setOutputHighOnPin( GPIO_PORT_P6 , GPIO_PIN0 );
                    GPIO_setOutputLowOnPin( GPIO_PORT_P6 , GPIO_PIN1 );
                    while (GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6 ) == 0) {
                        timercount = 0;
                        while (timercount <= 5) ;
                        GPIO_toggleOutputOnPin(GPIO_PORT_P6, GPIO_PIN0);
                    }

                    // restarts game
                    GPIO_setOutputLowOnPin( GPIO_PORT_P6 , GPIO_PIN0 );
                    gamestart = 1;
                    round = 1;
                    patterncomplete = 0;
                }
            }
        }
    }
}



void GPIOInit() {

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7 );

    GPIO_setAsInputPin( GPIO_PORT_P5, GPIO_PIN6 );
    GPIO_setAsInputPin( GPIO_PORT_P3, GPIO_PIN5 );
    GPIO_setAsOutputPin( GPIO_PORT_P6, GPIO_PIN0 | GPIO_PIN1);
    GPIO_setAsOutputPin( GPIO_PORT_P3, GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin( GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);
    GPIO_setAsOutputPin( GPIO_PORT_P8, GPIO_PIN0 | GPIO_PIN5 );
    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN7 | GPIO_PIN6 );

    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 );

}

void Timer_Init() {

    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    config.timerPeriod = 18750;
    config.timerClear = TIMER_A_DO_CLEAR;
    config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Timer_ISR);
    Timer_A_configureUpMode(TIMER_A2_BASE, &config);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

}

void Timer_ISR() {
    Timer_A_clearInterruptFlag(TIMER_A2_BASE);
    timercount++;
}

uint8_t bumpers() {
    // returns 0-5 for bumpers #0-5, returns 6 if no bumpers pressed
    uint8_t BMP0 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0);
    uint8_t BMP1 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2);
    uint8_t BMP2 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3);
    uint8_t BMP3 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN5);
    uint8_t BMP4 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN6);
    uint8_t BMP5 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN7);

    if (BMP0 == 0) {
        __delay_cycles(240e3);
        // red
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN0 );
        while (BMP0 == 0) {
            BMP0 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0);
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN0 );
        return 0;
    }
    else if (BMP1 == 0) {
        __delay_cycles(240e3);
        // green
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN1 );
        while (BMP1 == 0) {
            BMP1 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2);
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN1 );
        return 1;
    }
    else if (BMP2 == 0) {
        __delay_cycles(240e3);
        // blue
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN2 );
        while (BMP2 == 0) {
            BMP2 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3);
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN2 );
        return 2;
    }
    else if (BMP3 == 0) {
        __delay_cycles(240e3);
        // yellow
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN0 );
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN1 );
        while (BMP3 == 0) {
            BMP3 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN5);
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN0 );
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN1 );
        return 3;
    }
    else if (BMP4 == 0) {
        __delay_cycles(240e3);
        // purple
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN0 );
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN2 );
        while (BMP4 == 0) {
            BMP4 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN6);
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN0 );
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN2 );
        return 4;
    }
    else if (BMP5 == 0) {
        __delay_cycles(240e3);
        // cyan
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN2 );
        GPIO_setOutputHighOnPin( GPIO_PORT_P2 , GPIO_PIN1 );
        while (BMP5 == 0) {
            BMP5 = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN7);
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN2 );
        GPIO_setOutputLowOnPin( GPIO_PORT_P2 , GPIO_PIN1 );
        return 5;
    }
    return 6;
}
