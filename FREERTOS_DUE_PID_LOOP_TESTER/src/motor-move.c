/*
 * Motor-Move.c
 *
 * Created: 12/09/2020 8:21:34 p.m.
 *  Author: TCT
 */ 
#include "motor-move.h"
#include <gpio.h>
#include <ioport.h>

#define PERIOD_VALUE  100

#define MAX_SIZE_OF_TIME_ARRAY 8
int duty_cycle = 75;
TickType_t time_between_interrupts[MAX_SIZE_OF_TIME_ARRAY];
int TimeBetweenInterrupts_index = 0;
float Motor_Speed = 0;
char direction = 's';
int Motor_range = 0;

EventBits_t m_update_loop_flag = (1UL << 0);
EventBits_t m_position_loop_flag = (1UL << 1);
EventBits_t m_speed_loop_flag = (1UL << 2);
EventBits_t m_stats_capture_flag = (1UL << 3);

void motorBackward(){
	direction = 'b';
	ioport_set_pin_level(PIO_PC13_IDX, true);
	pwm_channel_update_duty(PWM,&pwm_channel_2_instance,PERIOD_VALUE - duty_cycle);
	pwm_channel_update_duty(PWM,&pwm_channel_3_instance,0);
}
void motorFoward(){
	direction = 'f';
	ioport_set_pin_level(PIO_PC13_IDX, true);
	pwm_channel_update_duty(PWM,&pwm_channel_2_instance,PERIOD_VALUE);
	pwm_channel_update_duty(PWM,&pwm_channel_3_instance,duty_cycle);
}
void motorStop(){
	direction = 's';
	pwm_channel_update_duty(PWM,&pwm_channel_2_instance,PERIOD_VALUE);
	pwm_channel_update_duty(PWM,&pwm_channel_3_instance,0);
	ioport_set_pin_level(PIO_PC13_IDX, false);
}
void setDutyCycle(int period){
	float x = PERIOD_VALUE/100 * period;
	if(x > PERIOD_VALUE){x = PERIOD_VALUE;}
	duty_cycle = x;
	switch(direction){
		
		case 's':
			motorStop();
			break;
		
		case 'f':
			motorFoward();
			break;
		
		case 'b':
			motorBackward();
			break;		
		
		default:
			break;
	}
}

void updateTimeBetweenInterrupts(TickType_t time){
	
	time_between_interrupts[TimeBetweenInterrupts_index % MAX_SIZE_OF_TIME_ARRAY] = (time >= 0)? time : 0;
	if (TimeBetweenInterrupts_index > 2* MAX_SIZE_OF_TIME_ARRAY)
	{
		TimeBetweenInterrupts_index = MAX_SIZE_OF_TIME_ARRAY;
	}
}
//nb doesn't include time from last interrupt.
TickType_t getAverageOfTimeBetweenInterrupts(){
	TickType_t out = 0;
	int numberofvalues = TimeBetweenInterrupts_index > MAX_SIZE_OF_TIME_ARRAY? MAX_SIZE_OF_TIME_ARRAY : TimeBetweenInterrupts_index;
	for (int i = 0; i < numberofvalues; i++)
	{
		out += time_between_interrupts[i];
	}
	
	return out/numberofvalues;
}

float GetSpeed( void ){
	return Motor_Speed/4;
}
void updateSpeed(int speed){
	Motor_Speed = speed;
}


char getDirection( void ){
	return direction;
}

int getRange( void ){
	if (Motor_range == 0)
	{
		return 1;
	}
	
	return Motor_range;
}
void updateRange(int range){
	Motor_range = range;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	for (;;) {
	}
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle pxTask,
		signed char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for (;;) {
	}
}

/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
}

/*-----------------------------------------------------------*/

void assert_triggered(const char *file, uint32_t line)
{
	volatile uint32_t block_var = 0, line_in;
	const char *file_in;

	/* These assignments are made to prevent the compiler optimizing the
	values away. */
	file_in = file;
	line_in = line;
	(void) file_in;
	(void) line_in;

	taskENTER_CRITICAL();
	{
		while (block_var == 0) {
			/* Set block_var to a non-zero value in the debugger to
			step out of this function. */
		}
	}
	taskEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/
