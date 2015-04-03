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
		if(READ)
		{
			status[pi] = !status[pi];
			doors = status[8];
			if(status[pi])
			{
				if(pi==3)
						setCarMotorStopped(1);
				if(pi==0 || pi ==1 || pi == 2)
				{
					
					if(!inQueue(pi))
					{
						for (i=0;i<3;i++) 
						{
							if (floorRequest[i] == -1) 
							{
								floorRequest[i] = Floors[pi];
								printf("Queued: %d\n", pi);
								break;	
							}
						}
						if(SAFE_TO_STOP && pi == 1)
						{
							printf("Safe to make a stop at floor 2 first!\n");
							floorRequest[2] = floorRequest[1];
							floorRequest[1] = floorRequest[0];
							floorRequest[0] = Floors[pi];
							destination = Floors[pi];
							setCarTargetPosition(destination);
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
				if(floorRequest[0]!= -1 && reached && (count >= 210) && doors)
					{
						destination = floorRequest[0];
						setCarTargetPosition(destination);
						reached = false;
						printf("\nSetting car target position to: %d\n", destination);
					}
					
					if((currentPos >= destination - 0.5) && (currentPos <= destination + 0.5) && !reached)
					{
						floorRequest[0] = floorRequest[1];
						floorRequest[1] = floorRequest[2];
						floorRequest[2] = -1;
						reached = true;
						count=0;
						printf("\nReached destination: %lu\n", currentPos);
					}
			}
			if(reached)
			{
				if (count <= 210)
					count++;
			}

		vTaskDelayUntil(&xLastWakeTime, DELAY);
	}
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
