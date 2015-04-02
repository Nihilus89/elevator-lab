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

#include "global.h"
#include "planner.h"
#include "assert.h"
#include <stdbool.h>
#include "assert.h"

#define READ xQueueReceive(pinEventQueue, &pi, (portTickType)0)
#define BUTTON0 READ & (pi == 0)

static void plannerTask(void *params) {
	PinEvent pi;
	bool status[9] = {false};
	
	for(;;)
	{		
		if(READ)
		{
			if(pi == 0)
				//status[pi] = !status[pi];
				setCarTargetPosition(FLOOR_1);
			if(pi == 1)
				setCarTargetPosition(FLOOR_2);
			if(pi == 2)
				setCarTargetPosition(FLOOR_3);
			if(pi == 3)
				setCarMotorStopped(1);
			if(pi == 7)
				printf("AT_FLOOR\n");
			if(status[pi])
				printf("Button %d pressed\n", pi);
			//else
				//printf("Button %d released\n", pi);
			
		}
	}
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
