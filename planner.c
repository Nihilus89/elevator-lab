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


#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

#define SAFE_DISTANCE 5 // safe distance for test floor locations

#define READ xQueueReceive(pinEventQueue, &pi, (portTickType)0)

#define UPWARDS_SAFE MOTOR_UPWARD && (currentPos < (Floors[FLOOR_2] - SAFE_DISTANCE))
#define DOWNWARDS_SAFE MOTOR_DOWNWARD && (currentPos > (Floors[FLOOR_2] + SAFE_DISTANCE))
#define SAFE_TO_STOP UPWARDS_SAFE || DOWNWARDS_SAFE

int floorRequest[3]= {-1,-1,-1};
int Floors[3]= {0,20,40};
int inQueue(PinEvent pi);
s32 currentPos;
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
	char qHead=0,qTail=0;
	int destination, OLDdestination, interm_FLG = false;
	bool status[9] = {false}, reached = true, swap_FLG = false;
	
	for(;;)
	{
		currentPos = getCarPosition();
		if(READ)
		{
			status[pi] = !status[pi];
			if(status[pi])
			{
				if(pi==0 || pi ==1 || pi == 2)
				{
					
					if(!inQueue(pi))
					{
						if(qHead < 3)
						{
							floorRequest[qHead++] = Floors[pi];
							printf("Queued: %d\n", pi);
						}
						else
						{
							qHead = 0;
							floorRequest[qHead++] = Floors[pi];
							printf("Queued: %d\n", pi);
						}
					}
					else
						printf("%d is already queued\n", pi);
				}
			}
		}
		
		// Reading the queue and setting the destination
			if(MOTOR_STOPPED)
			{
				//if((currentPos >= (destination - 0.5)) && (currentPos <=(destination + 0.5)))
				//{	
					if(qTail > 2)
						qTail =0;
					
					if(floorRequest[qTail]!= -1 && reached)
					{
						destination = floorRequest[qTail];
						setCarTargetPosition(destination);
						reached = false;
						printf("\nSet car target position\n");
						printf("floorRequest[0]: %d\n", floorRequest[0]);
						printf("floorRequest[1]: %d\n", floorRequest[1]);
						printf("floorRequest[2]: %d\n", floorRequest[2]);
					}
					
					if((currentPos >= destination - 0.5) && (currentPos <= destination + 0.5) && !reached)
					{
						floorRequest[qTail] = -1;
						qTail++;
						reached = true;
						printf("\nReached destination\n");
						printf("floorRequest[0]: %d\n", floorRequest[0]);
						printf("floorRequest[1]: %d\n", floorRequest[1]);
						printf("floorRequest[2]: %d\n", floorRequest[2]);
					}
			}
			
			
			/*else
			{
				if((floorRequest[qTail] == 20) && !interm_FLG) // if floor request is 2 (somebody caled the elevator to floor 2)
				{
					if(MOTOR_UPWARD) // and your moving upwards
					{
						if(currentPos < (Floors[FLOOR_2] - SAFE_DISTANCE))	// if you are at a safe distance from floor 2 to make a stop
						{

								 //you make a stop at floor 2
								printf("\nDestination changed\n");
								OLDdestination = destination;
								destination = Floors[FLOOR_2];
								setCarTargetPosition(destination);
								interm_FLG = true;
							
						}
					}
					else if(MOTOR_DOWNWARD) // else if you are moving downwards
					{
						if(currentPos > (Floors[FLOOR_2] + SAFE_DISTANCE))	// if you are at a safe distance from floor 2 to make a stop
						{

							// you make a stop at floor 2
							printf("\nDestination changed\n");
							OLDdestination = destination;
							destination = Floors[FLOOR_2];
							setCarTargetPosition(destination);
							interm_FLG = true;
							
						}
					}
				}
			}
			
		if(interm_FLG && MOTOR_STOPPED)
		{
			setCarTargetPosition(OLDdestination);
			interm_FLG = false;
		}
		
		
	//getCarPosition();
	// SAFE_DISTANCE
						
			if(pi == 0)
				setCarTargetPosition(FLOOR_1);
			if(pi == 1)
				setCarTargetPosition(FLOOR_2);
			if(pi == 2)
				setCarTargetPosition(FLOOR_3);
			if(pi == 3)
				setCarMotorStopped(1);
			if(pi == 7)
				printf("AT_FLOOR\n");*/
			//if(status[pi])
				//printf("Button %d pressed\n", pi);
			//else
				//printf("Button %d released\n", pi);
		
	}
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
