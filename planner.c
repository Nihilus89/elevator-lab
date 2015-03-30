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

#define READ xQueueReceive(pinEventQueue, &pi, (portTickType)0)
#define BUTTON0 READ & (pi == 0)

static void plannerTask(void *params) {
	PinEvent pi;
	bool status = false, newstate = false;
	
	for(;;)
	{
		if((READ) != status)
		{
			vTaskDelay(20/ portTICK_RATE_MS);
			newstate = READ;
			if(newstate != status)
			{
				if(!status)
					printf("Button %d pressed\n", pi);
				else
					printf("Button %d released\n", pi);
				status = newstate;
				
			}
		}

	}
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
