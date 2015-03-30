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



static void pollPin(PinListener *listener, xQueueHandle pinEventQueue)
{
	int pin;
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0))
	{
		pin = 0;
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1))
	{
		pin = 1;
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2))
	{
		pin = 2;
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3))
	{
		pin = 3;
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7))
	{
		pin = 7;
		xQueueSend(pinEventQueue, &pin, portMAX_DELAY); 
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8))
	{
		pin = 8;
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
