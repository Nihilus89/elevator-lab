/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The planner module, which is responsible for consuming
 * pin/key events, and for deciding where the elevator
 * should go next
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "stm32f10x_tim.h"
#include "global.h"
#include "planner.h"
#include "assert.h"
#include "queue.h"
#include <stdbool.h>

#define DELAY (5 / portTICK_RATE_MS)
#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

#define SAFE_DISTANCE 50 // safe distance for test floor locations

#define READ xQueueReceive(pinEventQueue, &pi, (portTickType)0)

#define UPWARDS_SAFE MOTOR_UPWARD && (currentPos < (Floors[FLOOR_2] - SAFE_DISTANCE))
#define DOWNWARDS_SAFE MOTOR_DOWNWARD && (currentPos > (Floors[FLOOR_2] + SAFE_DISTANCE))
#define SAFE_TO_STOP (UPWARDS_SAFE || DOWNWARDS_SAFE)

int floorRequest[3]= {-1,-1,-1};
int Floors[3]= {0,400,800};
int inQueue(PinEvent pi);
s32 currentPos;
static portTickType xLastWakeTime;
int inQueue(PinEvent pi)
{
	int i;

	for(i=0;i<3;i++)
	{
		if(floorRequest[i]==Floors[pi])
			return 1;
	}
	return 0;
}

static void plannerTask(void *params) {
	
	PinEvent pi;
	int destination, count = 0, i;
	bool status[9] = {false}, reached = true, doors;
	
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		currentPos = getCarPosition();
		if(READ) // if a key press has been polled
		{
			status[pi] = !status[pi];	// status = 1 pressed, status 0 = released for each pin
			doors = status[8];	// get the status of the door
			if(status[pi])		// process presses of the buttons
			{
				if(pi==3)	// just stop the car in case of pin 3 press
						setCarMotorStopped(1);
				if(pi==0 || pi ==1 || pi == 2)	// floor buttons
				{
					
					if(!inQueue(pi))	// check if it's present in the queue
					{
						for (i=0;i<3;i++) 
						{
							if (floorRequest[i] == -1) // -1 is the "empty" value
							{
								floorRequest[i] = Floors[pi];	// when you find the next empty value, insert and break the loop
								printf("Queued: %d\n", pi);
								break;	
							}
						}
						
						// if the elevator is already moving and gets a call from floor 2 within a safe distance, make a stop there first
						if(SAFE_TO_STOP && pi == 1)	// macro determining if it's safe, and only evaluate for press of floor 2
						{
							printf("Safe to make a stop at floor 2 first!\n");
							floorRequest[2] = floorRequest[1];
							floorRequest[1] = floorRequest[0];
							floorRequest[0] = Floors[pi];
							destination = Floors[pi];		// bring the call to floor 2 in the front of the queue
							setCarTargetPosition(destination);	// set the destination to floor 2
						}
					}
					else
						printf("%d is already queued\n", pi);
					}
				}
			}
		
		
		// Reading the queue and setting the destination
			if(MOTOR_STOPPED)	// calls are processed when the elevator is not moving
			{				
				if(floorRequest[0]!= -1 && reached && (count >= 210) && doors)	// if there's something in the queue, the elevator has reached its destination and stayed for at least a second and the doors are closed
					{
						destination = floorRequest[0];
						setCarTargetPosition(destination);
						reached = false;
						printf("\nSetting car target position to: %d\n", destination);
					}
					
					if((currentPos >= destination - 0.5) && (currentPos <= destination + 0.5) && !reached)	// when you reach the destination, pop the first element
					{
						floorRequest[0] = floorRequest[1];
						floorRequest[1] = floorRequest[2];
						floorRequest[2] = -1;
						reached = true;
						count=0;
						printf("\nReached destination: %lu\n", currentPos);
					}
			}
			if(reached) // 210 * 5 ms minimum wait when reached a destination
			{
				if (count <= 210)	// no need to determine value after that point
					count++;
			}

		vTaskDelayUntil(&xLastWakeTime, DELAY);
	}
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
