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

#include "position_tracker.h"

#include "assert.h"
#include <stdbool.h>

#define POSITION GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)

static void positionTrackerTask(void *params) 
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = 3/portTICK_RATE_MS;
	bool pulse = false;
	
	PositionTracker *tracker = (PositionTracker*)params;
	xLastWakeTime = xTaskGetTickCount();
	
	for (;;) 
	{

		if (POSITION)
		{
			if(!pulse)
			{
				pulse = true;
				
				if (tracker->direction == Up)
					tracker->position++;
				else if( tracker->direction == Down )
					tracker->position--;
			}
		else
			pulse = false;
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		}
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

  // ...

}

s32 getPosition(PositionTracker *tracker) {

  // ...

  return 0;

}

