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
#define BUTTON0 READ & (pi == 0)
int floorRequest[3]= {-1,-1,-1};
int Floors[3]= {0,20,40};
int inQueue(PinEvent pi);
s32 currentPos;
int inQueue(PinEvent pi)
{
	int i;

	for(i=0;i<3;i++)
	{
		if(floorRequest[i]==pi)
			return 1;
	}
	return 0;
}

static void plannerTask(void *params) {
	PinEvent pi;
	 //xQueueHandle floorRequest;
	//int i;
	char qHead=0,qTail=0;
	int destination, OLDdestination, interm_FLG = false;
	 
	bool status[9] = {false};
	//floorRequest = xQueueCreate( 10, sizeof( PinEvent ) );
	for(;;)
	{
		currentPos = getCarPosition();
		if(READ)
		{
			printf("\npin event");
			status[pi] = !status[pi];
			if(status[pi])
			{
				
				// Queue the call if it's not already present
				if(pi==0 || pi ==1 || pi == 2)
				{
					
					if(!inQueue(pi))
					{
						if(qHead < 3)
						{
							floorRequest[qHead++] = Floors[pi];
							printf("\nqueued\n");
						}
						else
						{
							qHead = 0;
							floorRequest[qHead++] = Floors[pi];
							printf("\nqueued\n");
						}
					}
					else
						printf("\nalready queued\n");
				}
			}
		}
	
	
		
		// Reading the queue and setting the destination
			if(MOTOR_STOPPED )
			{
				//if((currentPos >= (destination - 0.5)) && (currentPos <=(destination + 0.5)))
				//{	
					if(qTail > 2)
						qTail =0;
					
					if(floorRequest[qTail]!= -1)
					{
						destination = floorRequest[qTail];
						setCarTargetPosition(destination);
						floorRequest[qTail] = -1;
						qTail++;
					}
				//}
				//else
					//setCarTargetPosition(destination);
			}
			else
			{
				if(floorRequest[1] != -1) // if floor request is 2 (somebody caled the elevator to floor 2)
				{
					if(MOTOR_UPWARD) // and your moving upwards
					{
						if(currentPos < (Floors[FLOOR_2] - SAFE_DISTANCE))	// if you are at a safe distance from floor 2 to make a stop
						{
							if(!interm_FLG)
							{
								 //you make a stop at floor 2
								printf("\nDestination changed\n");
								printf("\nfloorRequest[1]: %d\n", floorRequest[1]);
								OLDdestination = destination;
								destination = Floors[FLOOR_2];
								setCarTargetPosition(destination);
								interm_FLG = true;
							}
						}
					}
					else if(MOTOR_DOWNWARD) // else if you are moving downwards
					{
						if(currentPos > (Floors[FLOOR_2] + SAFE_DISTANCE))	// if you are at a safe distance from floor 2 to make a stop
						{
							if(!interm_FLG)
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
			}
			
		if(interm_FLG && MOTOR_STOPPED)
		{
			setCarTargetPosition(OLDdestination);
			interm_FLG = false;
		}
	//getCarPosition();
	// SAFE_DISTANCE
						
			/*if(pi == 0)
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
