
/*
 * Motor_Move.h
 *
 * Created: 12/09/2020 8:23:43 p.m.
 *  Author: TCT
 */ 
#ifndef MOTORMOVE_H
#define MOTORMOVE_H

#include <pwm.h>

// From module: FreeRTOS - kernel 10.0.0
#include <FreeRTOS.h>
//#include <StackMacros.h>
#include <croutine.h>
#include <deprecated_definitions.h>
#include <event_groups.h>
#include <list.h>
#include <message_buffer.h>
#include <mpu_wrappers.h>
#include <portable.h>
#include <projdefs.h>
#include <queue.h>
#include <semphr.h>
#include <stack_macros.h>
#include <stream_buffer.h>
#include <task.h>
#include <timers.h>

#define POSITION_DEADBAND 3
#define m_DEADBAND(X,Y,Z) ((X) <= (Y + Z) ? ((X) >= (Y - Z) ? (1) : (0)) : (0))
#define mp_DEADBAND(X,Y) m_DEADBAND(X,Y,POSITION_DEADBAND)
#define PWMMAX 100

QueueHandle_t motorque;

pwm_channel_t pwm_channel_2_instance;
pwm_channel_t pwm_channel_3_instance;

EventGroupHandle_t m_control_flags;

extern EventBits_t m_update_loop_flag;
extern EventBits_t m_position_loop_flag;
extern EventBits_t m_speed_loop_flag;
extern EventBits_t m_stats_capture_flag;

int A_interrupt;
int B_interrupt;
volatile int M_position;
int M_wanted_position;
float M_wanted_speed;
bool Motor_SpeedUpdate;

// Values of control loops
	volatile float speed_loop_pK;
	volatile float speed_loop_iK;
	volatile float speed_loop_dK;

	volatile float position_loop_pK;
	volatile float position_loop_iK;
	volatile float position_loop_dK;


typedef struct
{
	char direction;
	int run_time;
} motor_que_item;


void motorFoward(void);
void motorBackward(void);
void motorStop(void);
void setDutyCycle(int period);
void updateTimeBetweenInterrupts(TickType_t time);
//nb doesn't include time from last interrupt.
TickType_t getAverageOfTimeBetweenInterrupts(void);
float GetSpeed(void);
void updateSpeed(int speed);
char getDirection(void);
int getRange(void);
void updateRange(int range);
#endif