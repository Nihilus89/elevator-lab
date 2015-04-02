/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Functions listening for changes of specified pins
 */

#include "FreeRTOS.h"
#include "task.h"

#include "pin_listener.h"
#include "assert.h"
#include <stdbool.h>

int status[9] = {0};


static void pollPin(PinListener *listener, xQueueHandle pinEventQueue)
{
	int pin;
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) != status[0])
	{
		pin = 0;
		status[pin] = !status[pin];
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)!= status[1])
	{
		pin = 1;
		status[pin] = !status[pin];
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)!= status[2])
	{
		pin = 2;
		status[pin] = !status[pin];
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)!= status[3])
	{
		pin = 3;
		status[pin] = !status[pin];
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)!= status[7])
	{
		pin = 7;
		status[pin] = !status[pin];
		if(status[pin])
			xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)!= status[8])
	{
		pin = 8;
		status[pin] = !status[pin];
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
		
}

static void pollPinsTask(void *params) {
  PinListenerSet listeners = *((PinListenerSet*)params);
  portTickType xLastWakeTime;
  int i;

  xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    for (i = 0; i < listeners.num; ++i)
	  pollPin(listeners.listeners + i, listeners.pinEventQueue);
    
	vTaskDelayUntil(&xLastWakeTime, listeners.pollingPeriod);
  }
}

void setupPinListeners(PinListenerSet *listenerSet) {
  portBASE_TYPE res;

  res = xTaskCreate(pollPinsTask, "pin polling",
                    100, (void*)listenerSet,
					listenerSet->uxPriority, NULL);
  assert(res == pdTRUE);
}
