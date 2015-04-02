/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Class for keeping track of the car position.
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "position_tracker.h"

#include "assert.h"
#include <stdbool.h>
#include <stdlib.h>
#include "global.h"

#define POSITION GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)
#define MAXCOUNT 10000 // making sure the counter never overflows

#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)


// Speed violation flag
extern unsigned char SPEED_LIMIT;
static void positionTrackerTask(void *params) 
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = 3/portTICK_RATE_MS;
	bool pulse = false;
	
	unsigned int count = 0, pulses = 0, speed = 0, oldspeed = 0;
	
	PositionTracker *tracker = (PositionTracker*)params;
	xLastWakeTime = xTaskGetTickCount();
	
	for (;;) 
	{
		oldspeed = speed;		// just for the printout
		if (POSITION)
		{
			if(!pulse)
			{
				pulse = true;
				if (tracker->direction == Up)
					tracker->position++;
				else if( tracker->direction == Down )
					tracker->position--;
				if(!count)
					speed = 0;
				else
					speed = (pulses*1000) / (count*3);
				if(speed>49)
					SPEED_LIMIT = 0;
				else
					SPEED_LIMIT = 1;
				pulses++;
			}
		}
		else
			pulse = false;

	count++;
	
	if(count == MAXCOUNT || MOTOR_STOPPED)	// <- this changed, comment to show Sahil
	{
		count = 1;
		pulses = 0;
		speed = 0;	// <- this changed, comment to show Sahil
	}
	
	// speed debugging printout
	//if(oldspeed - speed)
		//printf("Current speed: %d cm/s\n", speed);
	vTaskDelayUntil(&xLastWakeTime, xFrequency);

	}

		
}

void setupPositionTracker(PositionTracker *tracker,
                          GPIO_TypeDef * gpio, u16 pin,
						  portTickType pollingPeriod,
						  unsigned portBASE_TYPE uxPriority) {
  portBASE_TYPE res;

  tracker->position = 0;
  tracker->lock = xSemaphoreCreateMutex();
  assert(tracker->lock != NULL);
  tracker->direction = Unknown;
  tracker->gpio = gpio;
  tracker->pin = pin;
  tracker->pollingPeriod = pollingPeriod;

  res = xTaskCreate(positionTrackerTask, "position tracker",
                    80, (void*)tracker, uxPriority, NULL);
  assert(res == pdTRUE);
}

void setDirection(PositionTracker *tracker, Direction dir) {

	tracker->direction = dir;

}

s32 getPosition(PositionTracker *tracker) {

  return tracker->position;

}
