/**
 * Stroker Pinout for Version 0.1 (using breakout boards)
 * File: <Stroker_pinout.h>
 * Auth: <Bart Garcia-Nathan> 					<<bart.garcia.nathan@gmail.com>>
 * Auth: <Jose Espinoza Mendoza> 				<<joseluisesp@gmail.com>>
 * Date: <Sep 2018>
 */
 
#ifndef STROKER_H
#define STROKER_H

//Stepper pins  -----------------------------------------------
#define STPR1				8
#define STPR2				10
#define STPR3				9
#define STPR4				11

#define Stepper_steps		2040

//Servo   ----------------------------------------------------
#define Servo_pin			6

//IR  --------------------------------------------------------
#define IR_pin				A4

//LEDs  ------------------------------------------------------
#define LED1				2
#define LED2				3
#define LED3				4
#define LED4				5

#define ON_LED				LED1
#define BUSY_LED			LED2
#define ERROR_LED			LED4
#define READY_LED			LED3

//Pushbuttons  -----------------------------------------------
#define SW1					A2 	//(active high)
#define SW2					7 	//(active high)
#define SW3					12	//(active high)

#define HOME_SW				SW1
#define START_SW			SW2
#define STOP_SW				SW3


#endif