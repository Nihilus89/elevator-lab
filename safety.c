/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * This file defines the safety module, which observes the running
 * elevator system and is able to stop the elevator in critical
 * situations
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>

#include "global.h"
#include "assert.h"

#define POLL_TIME (10 / portTICK_RATE_MS)

#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)
#define AT_FLOOR      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

#define PEEK xQueuePeek(pinEventQueue, &qPeek, (portTickType)0)
#define FLOOR_CALL_QUEUED  PEEK & ((qPeek == 0) || (qPeek == 1) || (qPeek == 2))

unsigned char SPEED_LIMIT;
static portTickType xLastWakeTime;

extern bool TargetSet;

PinEvent qPeek;

static void check(u8 assertion, char *name) {
  if (!assertion) {
    printf("SAFETY REQUIREMENT %s VIOLATED: STOPPING ELEVATOR\n", name);
    for (;;) {
	  setCarMotorStopped(1);
  	  vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
    }
  }
}

static void safetyTask(void *params) {
  s16 timeSinceStopPressed = -1;
	s32 position = 0;
	char FLOOR_POS = 1, rightPosition = 1;
	//portTickType motor_started = 0, OLD_motor_started = 0;
	s16 motor_flg = 0, arrived = 1;
	unsigned long on_floor = 0;
	
	
  xLastWakeTime = xTaskGetTickCount();
	SPEED_LIMIT = 1;

	
  for (;;) {
		
		//******* Debugging only: can be ignored********************************
		
		
		if((!MOTOR_STOPPED) && !motor_flg) // if motor is running and the motor flag isn't raised
		{
			motor_flg = 1;
			printf("Motor STARTED\n");
		}
		else if(MOTOR_STOPPED && motor_flg)
		{
			printf("Motor STOPPED\n");
			motor_flg = 0;
		}
		//**********************************************************************
		
		
    // Environment assumption 1: the doors can only be opened if
	//                           the elevator is at a floor and
    //                           the motor is not active
	check((AT_FLOOR && MOTOR_STOPPED) || DOORS_CLOSED, "env1");

	// Environment assumption 2 : Maximum speed is 50 cm/s
	
	check(SPEED_LIMIT, "env2");
	
		/* Environment assumption 3 : If the ground floor is put at 0cm in an absolute coordinate system, 
		the second floor is at 400cm and the third floor at 800cm (the at-floor sensor reports a floor 
		with a threshold of +-0.5cm)*/
		position = getCarPosition();
		FLOOR_POS = 1;
		//rightPosition = ((position == FLOOR_1 || position == FLOOR_1 + 1 || position == FLOOR_2 - 1 || position == FLOOR_2 || position == FLOOR_2 + 1 || position == FLOOR_3 - 1 || position == FLOOR_3 || position == FLOOR_3 + 1));
		rightPosition = ((position == FLOOR_1 || position == FLOOR_1 + 1 || position == 400 - 1 || position == 400 || position == 400 + 1 || position == 800 - 1 || position == 800 || position == 800 + 1));
		if(AT_FLOOR && MOTOR_STOPPED)
		{
			if(!rightPosition)
				FLOOR_POS = 0;
		}
		check(FLOOR_POS, "env3");

	// fill in your own environment assumption 4
	check(1, "env4");

    // System requirement 1: if the stop button is pressed, the motor is stopped within 1s

	if (STOP_PRESSED) {
	  if (timeSinceStopPressed < 0)
	    timeSinceStopPressed = 0;
      else
	    timeSinceStopPressed += POLL_TIME;

      check(timeSinceStopPressed * portTICK_RATE_MS <= 1000 || MOTOR_STOPPED,
	        "req1");
	} else {
	  timeSinceStopPressed = -1;
	}

    // System requirement 2: the motor signals for upwards and downwards movement are not active at the same time

    check(!MOTOR_UPWARD || !MOTOR_DOWNWARD,
          "req2");

	// System requirement 3: The elevator may not pass the end positions, that is, go through the roof or the floor
	check((position >= FLOOR_1 && position <=801), "req3");

	// System requirement 4: A moving elevator halts only if the stop button is pressed or the elevator has arrived at a floor
	if(MOTOR_STOPPED)
		check((STOP_PRESSED || AT_FLOOR), "req4");
	


	// System requirement 5: Once the elevator has stopped at a floor, it will wait for at least 1s before it continues to another floor
	if(MOTOR_STOPPED && AT_FLOOR)
	{
		if(arrived)
		{
			on_floor = 0;
			arrived = 0;
		}
		if (on_floor <= 100)
			on_floor++;
	}
	else
		arrived = 1;
	
	
		check(on_floor>=100 || MOTOR_STOPPED, "req5");

	// 	//System Requirement 6: Elevator is moving only if the target destination is set
	/*MOTOR_STOPPED		TargetSet  Output
				0							0					0						//CASE WHERE MOTOR IS RUNNING AND TARGET IS NOT SET
				0							1					1
				1							0					1
				1							1					1
	*/
	check((MOTOR_STOPPED^TargetSet)+(MOTOR_STOPPED&&TargetSet), "req6");

		
	vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
  }

}

void setupSafety(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(safetyTask, "safety", 100, NULL, uxPriority, NULL);
}

