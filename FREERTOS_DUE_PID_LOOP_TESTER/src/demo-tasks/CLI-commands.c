/**
 *
 * \file
 *
 * \brief FreeRTOS+CLI command examples
 *
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <ctype.h>

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

#include "demo-tasks.h"

/* Motor includes */
#include "mail_box.h"

/* Parser Table */
#include "parser/loopcommands.tab.h"
#include "tiny-gcc/re.h"
#include "parser/loopcommands.h"

/*Status Command */
#include "status-capture/status-capture.h"

/*
 * Implements the run-time-stats command.
 */
static portBASE_TYPE task_stats_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString);

/*
 * Implements the task-stats command.
 */
static portBASE_TYPE run_time_stats_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString);

/*
 * Implements the manipulate-pins command.
 */
static portBASE_TYPE manipulate_pins_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString);

/*
 * Implements the motor-forward command.
 */
static portBASE_TYPE motor_forward_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString);
		
/*
 * Implements the motor-backward command.
 */
static portBASE_TYPE motor_backward_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString);
		
/*
* Implements the set-duty-cycle command.
*/
static portBASE_TYPE set_duty_cycle_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString);
		
/*
* Implements the report-position command.
*/
static portBASE_TYPE report_position_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString);

/*
* Implements the range command.
*/
static portBASE_TYPE range_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString);

/*
* Implements the zero-position command.
*/
static portBASE_TYPE zero_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString);

/*
* Implements the control loop command.
*/
static portBASE_TYPE loop_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString);

/*
* Implements the command to print out stats  from stats buffer.
*/
static portBASE_TYPE printStats_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString);
		
/*
 * The task that is created by the create-task command.
 */
void created_task(void *pvParameters);

/*
 * Holds the handle of the task created by the create-task command.
 */
static xTaskHandle created_task_handle = NULL;

/* Structure that defines the "run-time-stats" command line command.
This generates a table that shows how much run time each task has */
static const CLI_Command_Definition_t run_time_stats_command_definition =
{
	(const int8_t *const) "run-time-stats", /* The command string to type. */
	(const int8_t *const) "run-time-stats:\r\n Displays a table showing how much processing time each FreeRTOS task has used\r\n\r\n",
	run_time_stats_command, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t task_stats_command_definition =
{
	(const int8_t *const) "task-stats", /* The command string to type. */
	(const int8_t *const) "task-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n\r\n",
	task_stats_command, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "loop_status" command line command.  This Lets 
the user turn on and off the PWM and enable pins that will be used by the motor driver, 
as well as turn on and off the HALL sensor interrupts. */
static const CLI_Command_Definition_t manipulate_pins_command_definition =
{
	(const int8_t *const) "manipulate-pins",
	(const int8_t *const) "manipulate-pins:\r\n \
	Turn on/off Motor driver pins\r\n \
	MDP1 on/off (Turn on off Motor Driver Pin 1) \r\n \
	MDP2 on/off (Turn on off Motor Driver Pin 2) \r\n \
	MDPWM1 on/off (Turn on off Motor Driver PWM Pin 1) \r\n \
	MDPWM2 on/off (Turn on off Motor Driver PWM Pin 2) \r\n  \
	MDHSA on/off (Turn on off Motor Hall sensor  A interrupt) \r\n  \
	MDHSB on/off (Turn on off Motor Hall sensor  N interrupt) \r\n\r\n",
	manipulate_pins_command, /* The function to run. */
	-1 /* The user can enter any number of commands. */
};

/* Structure that defines the "motor-forward" command line command.  This takes a
single parameter that is passed into a newly created task.  The task then
drives the motor forward for X seconds. */
static const CLI_Command_Definition_t motor_forward_command_definition =
{
	(const int8_t *const) "motor-forward",
	(const int8_t *const) "motor-forward <time ms>:\r\n  Drive motor forward X seconds.\r\n\r\n",
	motor_forward_command, /* The function to run. */
	1 /* A single parameter should be entered. */
};

/* Structure that defines the "motor-backward" command line command.  This takes a
single parameter that is passed into a newly created task.  The task then
drives the motor forward for X seconds. */
static const CLI_Command_Definition_t motor_backward_command_definition =
{
	(const int8_t *const) "motor-backward",
	(const int8_t *const) "motor-backward <time ms>:\r\n  Drive motor backward X seconds.\r\n\r\n",
	motor_backward_command, /* The function to run. */
	1 /* A single parameter should be entered. */
};
/* Structure that defines the "set-duty-cycle" command line command.  This takes a
single parameter that is converted to a % of duty cycle, will round down to 100%. */
static const CLI_Command_Definition_t set_duty_cycle_command_defintion =
{
	(const int8_t *const) "set-duty-cycle",
	(const int8_t *const) "set-duty-cycle <% of duty cycle>:\r\n  Set duty cycle to X%.\r\n\r\n",
	set_duty_cycle_command, /* The function to run. */
	1 /* A single parameter should be entered. */
};

/* Structure that defines the "report-position" command line command.  This takes a
no parameters and returns motor position and number of A & B Hall interrupts. */
static const CLI_Command_Definition_t report_position_command_defintion =
{
	(const int8_t *const) "motor-stats",
	(const int8_t *const) "motor-stats\r\n  Prints motor-stats.\r\n\r\n",
	report_position_command, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "range" command line command.  This takes a
no parameters and finds the motors range. */
static const CLI_Command_Definition_t range_command_defintion =
{
	(const int8_t *const) "range",
	(const int8_t *const) "range\r\n  Finds the motors range\r\n\r\n",
	range_command, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "zero" command line command.  This takes a
no parameters and zeros the motor. */
static const CLI_Command_Definition_t zero_command_defintion =
{
	(const int8_t *const) "zero",
	(const int8_t *const) "zero\r\n  Zeros the motor\r\n\r\n",
	zero_command, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "loop" command line command.  This takes a
no parameters and finds the motors range. */
static const CLI_Command_Definition_t loop_command_defintion =
{
	(const int8_t *const) "loop_command",
	(const int8_t *const) "loop_command on/off / loop_id xk value\r\n  Turn on and off control loops, and set there pid values.\r\n\r\n",
	loop_command, /* The function to run. */
	-1 /* Any number of parameters can be entered. */
};

/* Structure that defines the "loop_status" command line command.  This takes a
one parameters and prints x status. */
static const CLI_Command_Definition_t loop_status_command_defintion =
{
	(const int8_t *const) "loop_status",
	(const int8_t *const) "loop_status amount / loop_status amount\r\n  Print amount of stats latest first.\r\n\r\n",
	printStats_command, /* The function to run. */
	1 /* One parameter can be entered. */
};
/*-----------------------------------------------------------*/

void vRegisterCLICommands(void)
{
	/* Register all the command line commands defined immediately above. */
	FreeRTOS_CLIRegisterCommand(&manipulate_pins_command_definition);
	FreeRTOS_CLIRegisterCommand(&motor_forward_command_definition);
	FreeRTOS_CLIRegisterCommand(&motor_backward_command_definition);
	FreeRTOS_CLIRegisterCommand(&set_duty_cycle_command_defintion);
	FreeRTOS_CLIRegisterCommand(&report_position_command_defintion);
	FreeRTOS_CLIRegisterCommand(&range_command_defintion);
	FreeRTOS_CLIRegisterCommand(&zero_command_defintion);
	FreeRTOS_CLIRegisterCommand(&loop_command_defintion);
	FreeRTOS_CLIRegisterCommand(&loop_status_command_defintion);
}

/*-----------------------------------------------------------*/

static portBASE_TYPE task_stats_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	const int8_t *const task_table_header = (int8_t *) "Task          State  Priority  Stack	#\r\n************************************************\r\n";

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	/* Generate a table of task stats. */
	strcpy((char *) pcWriteBuffer, (char *) task_table_header);
	vTaskList(pcWriteBuffer + strlen((char *) task_table_header));

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE run_time_stats_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	const int8_t *const stats_table_header = (int8_t *) "Task            Abs Time      % Time\r\n****************************************\r\n";

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	/* Generate a table of task stats. */
	strcpy((char *) pcWriteBuffer, (char *) stats_table_header);
	vTaskGetRunTimeStats(pcWriteBuffer + strlen(
			(char *) stats_table_header));

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE three_parameter_echo_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	if (parameter_number == 0) {
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf((char *) pcWriteBuffer,
				"The three parameters were:\r\n");

		/* Next time the function is called the first parameter will be echoed
		back. */
		parameter_number = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		return_value = pdPASS;
	} else {
		/* Obtain the parameter string. */
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter
									(
										pcCommandString,		/* The command string itself. */
										parameter_number,		/* Return the next parameter. */
										&parameter_string_length	/* Store the parameter string length. */
									);

		/* Sanity check something was returned. */
		configASSERT(parameter_string);

		/* Return the parameter string. */
		memset(pcWriteBuffer, 0x00, xWriteBufferLen);
		sprintf((char *) pcWriteBuffer, "%ld: ", parameter_number);
		strncat((char *) pcWriteBuffer, (const char *) parameter_string,
				parameter_string_length);
		strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));

		/* If this is the last of the three parameters then there are no more
		strings to return after this one. */
		if (parameter_number == 3L) {
			/* If this is the last of the three parameters then there are no more
			strings to return after this one. */
			return_value = pdFALSE;
			parameter_number = 0L;
		} else {
			/* There are more parameters to return after this one. */
			return_value = pdTRUE;
			parameter_number++;
		}
	}

	return return_value;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE multi_parameter_echo_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	if (parameter_number == 0) {
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf((char *) pcWriteBuffer, "The parameters were:\r\n");

		/* Next time the function is called the first parameter will be echoed
		back. */
		parameter_number = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		return_value = pdPASS;
	} else {
		/* Obtain the parameter string. */
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter
									(
										pcCommandString,		/* The command string itself. */
										parameter_number,		/* Return the next parameter. */
										&parameter_string_length	/* Store the parameter string length. */
									);

		if (parameter_string != NULL) {
			/* Return the parameter string. */
			memset(pcWriteBuffer, 0x00, xWriteBufferLen);
			sprintf((char *) pcWriteBuffer, "%ld: ", parameter_number);
			strncat((char *) pcWriteBuffer, (const char *) parameter_string, parameter_string_length);
			strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));

			/* There might be more parameters to return after this one. */
			return_value = pdTRUE;
			parameter_number++;
		} else {
			/* No more parameters were found.  Make sure the write buffer does
			not contain a valid string. */
			pcWriteBuffer[0] = 0x00;

			/* No more data to return. */
			return_value = pdFALSE;

			/* Start over the next time this command is executed. */
			parameter_number = 0;
		}
	}

	return return_value;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE create_task_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length;
	static const int8_t *success_message = (int8_t *) "Task created\r\n";
	static const int8_t *failure_message = (int8_t *) "Task not created\r\n";
	static const int8_t *task_already_created_message = (int8_t *) "The task has already been created. Execute the delete-task command first.\r\n";
	int32_t parameter_value;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	/* Obtain the parameter string. */
	parameter_string = (int8_t *) FreeRTOS_CLIGetParameter(
									pcCommandString,		/* The command string itself. */
									1,						/* Return the first parameter. */
									&parameter_string_length	/* Store the parameter string length. */
								);

	/* Turn the parameter into a number. */
	parameter_value = (int32_t) atol((const char *) parameter_string);

	/* Attempt to create the task. */
	if (created_task_handle != NULL) {
		strcpy((char *) pcWriteBuffer,
				(const char *) task_already_created_message);
	} else {
		if (xTaskCreate(created_task, (const signed char *) "Created",
				configMINIMAL_STACK_SIZE,
				(void *) parameter_value, tskIDLE_PRIORITY,
				&created_task_handle) == pdPASS) {
			strcpy((char *) pcWriteBuffer,
					(const char *) success_message);
		} else {
			strcpy((char *) pcWriteBuffer,
					(const char *) failure_message);
		}
	}

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE delete_task_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	static const int8_t *success_message = (int8_t *) "Task deleted\r\n";
	static const int8_t *failure_message = (int8_t *) "The task was not running.  Execute the create-task command first.\r\n";

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	/* See if the task is running. */
	if (created_task_handle != NULL) {
		vTaskDelete(created_task_handle);
		created_task_handle = NULL;
		strcpy((char *) pcWriteBuffer, (const char *) success_message);
	} else {
		strcpy((char *) pcWriteBuffer, (const char *) failure_message);
	}

	/* There is no more data to return after this single string, so return
	 * pdFALSE. */
	return pdFALSE;
}

/*-----------------------------------------------------------*/

void created_task(void *pvParameters)
{
	int32_t parameter_value;
	static uint8_t local_buffer[60];

	/* Cast the parameter to an appropriate type. */
	parameter_value = (int32_t)pvParameters;

	memset((void *) local_buffer, 0x00, sizeof(local_buffer));
	sprintf((char *) local_buffer,
			"Created task running.  Received parameter %ld\r\n\r\n",
			(long) parameter_value);

	/* Cannot yet tell which CLI interface is in use, but both output functions
	guard check the port is initialised before it is used. */
#if (defined confINCLUDE_USART_CLI)
	usart_cli_output(local_buffer);
#endif

#if (defined confINCLUDE_CDC_CLI)
	cdc_cli_output(local_buffer);
#endif

	for (;;) {
		vTaskDelay(portMAX_DELAY);
	}
}

/*-----------------------------------------------------------*/
//Enum for command States
	typedef enum command_State {
		WAITING,
		MDP1,
		MDP2,
		MDPWM1,
		MDPWM2,
		MDHSA,
		MDHSB,
		CS_ON,
		CS_OFF,
		UNKNOWN,
		RECIVEDNAME,
		START,
		CS_TOGGLE,
		CS_END
		} command_State_t;
		
	typedef struct { const char *key; command_State_t val; } t_symstruct;
	static t_symstruct lookuptable[] = {
			{ "MDP1", MDP1 }, { "MDP2", MDP2 }, { "MDPWM1", MDPWM1 }, { "MDPWM2", MDPWM2 }, { "MDHSA", MDHSA }, { "MDHSB", MDHSB }, { "ON", CS_ON }, { "OFF", CS_OFF }
	};
	
	#define NKEYS (sizeof(lookuptable)/sizeof(t_symstruct))
	command_State_t keyfromstring(char *key);
	command_State_t keyfromstring(char *key)
	{
		
		for (unsigned int i=0; i < NKEYS; i++) {
			t_symstruct *sym = &lookuptable[i];
			if (strcmp(sym->key, key) == 0)
			return sym->val;
		}
		return UNKNOWN;
	};
	
	void stringfromval (command_State_t val,char *string);
	void stringfromval (command_State_t val,char *string)
		{
			
			for (unsigned int i=0; i < NKEYS; i++) {
				t_symstruct *sym = &lookuptable[i];
				if (sym->val == val){
					string = sym->key;
					return;
				}
				
			}
			return;
		};
	
	void run_arguments(command_State_t command, int8_t *pcWriteBuffer );
static portBASE_TYPE manipulate_pins_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;
	static command_State_t command = WAITING;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	if (parameter_number == 0) {
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf((char *) pcWriteBuffer, "The parameters were:\r\n");

		/* Next time the function is called the first parameter will be echoed
		back. */
		parameter_number = 1L;
		
		/* Set up the State Machine */
		command = START;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		return_value = pdPASS;
	} else {
		/* Obtain the parameter string. */
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter
									(
										pcCommandString,		/* The command string itself. */
										parameter_number,		/* Return the next parameter. */
										&parameter_string_length	/* Store the parameter string length. */
									);

		if (parameter_string != NULL) {
			/* Return the parameter string. */
			memset(pcWriteBuffer, 0x00, xWriteBufferLen);
			sprintf((char *) pcWriteBuffer, "%ld: ", parameter_number);
			strncat((char *) pcWriteBuffer, (const char *) parameter_string, parameter_string_length);
			strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));
			
			/* set string to all upper case */
			for(int i = 0; i < parameter_string_length; i++){
				parameter_string[i] = toupper(parameter_string[i]);
			}
			strncat((char *) pcWriteBuffer, "passed all upper case \r\n", strlen("passed all upper case \r\n"));
			command = keyfromstring((char *)parameter_string);
			strncat((char *) pcWriteBuffer, "got key \r\n", strlen("got key \r\n"));
			/* There might be more parameters to return after this one. */
			return_value = pdTRUE;
			parameter_number++;
			
			//
		} else {
			/* No more parameters were found.  Make sure the write buffer does
			not contain a valid string. */
			pcWriteBuffer[0] = 0x00;

			/* No more data to return. */
			return_value = pdFALSE;

			/* Start over the next time this command is executed. */
			parameter_number = 0;
			
			/* Set Up Ending State */
			command = CS_END;
		}
	}
	strncat((char *) pcWriteBuffer, "about to run argument \r\n", strlen("about to run argument \r\n"));
	run_arguments(command, pcWriteBuffer);
	strncat((char *) pcWriteBuffer, "ran argument \r\n", strlen("ran argument \r\n"));
	return return_value;
}

bool check_if_pin(command_State_t command);
bool check_if_pin(command_State_t command){
	bool out;
	switch(command){
		case MDP1 :
		case MDP2 :
		case MDPWM1 :
		case MDPWM2 :
		case MDHSA :
		case MDHSB :
			out = true;
			break;
		default:
			out = false;
	}
	return out;
}
void change_pin(command_State_t pin, command_State_t change);
void change_pin(command_State_t pin, command_State_t change){
		switch(pin){
			case MDP1 :
				if (change == CS_ON)
				{
					
				} 
				else if(change == CS_OFF)
				{
					
				}
				else if (change == CS_TOGGLE)
				{
					
				}
				break;
			case MDP2 :
			
				break;
			case MDPWM1 :
			
				break;
			case MDPWM2 :
			
				break;
			case MDHSA :
			
				break;
			case MDHSB :
			
			break;
			default:
			break;
		}
		return;
}


void run_arguments(command_State_t command, int8_t *pcWriteBuffer )
{
	static command_State_t state;
	if(command == START){
		state = WAITING;
		return;
	}
	if(state == WAITING){
		if(command == UNKNOWN){
			/* state stays waiting print error*/
			return;
		}
		if (command == CS_ON || command == CS_OFF)
		{
			/*print message have to chose a pin*/
			return;
		}
		if (check_if_pin(command))
		{
			state = command;
			return;
		}
		if (command == CS_END)
		{
			return;
		}
	}
	if(state!= WAITING){
		if(command == UNKNOWN){
			/* state = waiting print error*/
			state = WAITING;
			return;
		}
		if (command == CS_ON || command == CS_OFF)
		{
			/*print message set pin*/
			change_pin(state, command);
			state = WAITING;
			return;
		}
		if (check_if_pin(command))
		{
			/* toggle pin*/
			change_pin(state, CS_TOGGLE);
			state = command;
			return;
		}
		if (command == CS_END)
		{
			/* toggle pin*/
			change_pin(state, CS_TOGGLE);
			state = WAITING;
			return;
		}
	}
	return;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE motor_forward_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;
	static TickType_t run_time;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	motor_que_item motor_command;
	
	
	if(parameter_number == 0){
		/* Get run time */
		parameter_number++;
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter (
																pcCommandString,		/* The command string itself. */
																parameter_number,		/* Return the next parameter. */
																&parameter_string_length	/* Store the parameter string length. */
																);
		
		memset(pcWriteBuffer, 0x00, xWriteBufferLen);
		run_time = atoi((const char *)parameter_string);	
		sprintf((char *) pcWriteBuffer, "Running forward for: %ld ms , Param string length: %ld\r\n",run_time,parameter_string_length);
		strncat((char *) pcWriteBuffer, (const char *) parameter_string, parameter_string_length);
		strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));
		
		return_value = pdTRUE;
		motor_command.direction = 'f';
		motor_command.run_time = run_time;
		xQueueSendToBack( motorque,
		(const void *) &motor_command,
		0);
	} else {
		/* No more parameters were found.  Make sure the write buffer does
		not contain a valid string. */
		pcWriteBuffer[0] = 0x00;
		parameter_number = 0;
		return_value = pdFALSE;

	}
	
	
	return return_value;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE motor_backward_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;
	static TickType_t run_time;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	motor_que_item motor_command;
	
	if(parameter_number == 0){
		/* Get run time */
		parameter_number++;
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter (
																pcCommandString,		/* The command string itself. */
																parameter_number,		/* Return the next parameter. */
																&parameter_string_length	/* Store the parameter string length. */
																);
		
		memset(pcWriteBuffer, 0x00, xWriteBufferLen);
		run_time = atoi((const char *)parameter_string);	

		sprintf((char *) pcWriteBuffer, "Running backward for: %ld ms , Param string length: %ld\r\n",run_time,parameter_string_length);
		strncat((char *) pcWriteBuffer, (const char *) parameter_string, parameter_string_length);
		strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));
		
		return_value = pdTRUE;
		motor_command.direction = 'b';
		motor_command.run_time = run_time;
		xQueueSendToBack( motorque,
		(const void *) &motor_command,
		0);
	} else {
		/* No more parameters were found.  Make sure the write buffer does
		not contain a valid string. */
		pcWriteBuffer[0] = 0x00;
		parameter_number = 0;
		return_value = pdFALSE;
	}
	
	
	return return_value;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE set_duty_cycle_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	
	if(parameter_number == 0){
		/* Get run time */
		parameter_number++;
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter (
																pcCommandString,		/* The command string itself. */
																parameter_number,		/* Return the next parameter. */
																&parameter_string_length	/* Store the parameter string length. */
																);
		

		sprintf((char *) pcWriteBuffer, "Setting Duty cycle for motor: %d \r\n",atoi((const char *) parameter_string));
		setDutyCycle(atoi((const char *)parameter_string));
		return_value = pdTRUE;
	} else {
		/* No more parameters were found.  Make sure the write buffer does
		not contain a valid string. */
		pcWriteBuffer[0] = 0x00;
		parameter_number = 0;
		return_value = pdFALSE;
	}
	
	
	return return_value;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE report_position_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString){
	//int8_t *parameter_string;
	portBASE_TYPE return_value;
	static portBASE_TYPE parameter_number = 0;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	
	if(parameter_number == 0){
		parameter_number++;
		sprintf((char *) pcWriteBuffer, "Position: %d, A interrupts: %d, B interrupts: %d, Speed: %.3f, Range: %d \r\n \
         Wanted Position: %d Wanted Speed: %d \r\n",M_position, A_interrupt, B_interrupt, GetSpeed(),getRange(),M_wanted_position,M_wanted_speed);
		

		return_value = pdTRUE;
	} else if(parameter_number == 1){
		parameter_number++;
		sprintf((char *) pcWriteBuffer, " PID Loop Values: \r\n \
		Position Loop: pK: %.3f, iK %.3f, dK %.3f \r\n \
		Speed Loop: pK: %.3f, iK %.3f, dK %.3f \r\n ", position_loop_pK, position_loop_iK, position_loop_dK, speed_loop_pK, speed_loop_iK, speed_loop_dK );
		return_value = pdTRUE;
	} else {
		/* No more parameters were found.  Make sure the write buffer does
		not contain a valid string. */
		pcWriteBuffer[0] = 0x00;
		parameter_number = 0;
		return_value = pdFALSE;
	}
	
	
	return return_value;
}

/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/

static portBASE_TYPE zero_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString){
	//int8_t *parameter_string;
	portBASE_TYPE return_value;
	static portBASE_TYPE parameter_number = 0;
	motor_que_item motor_command;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	
	if(parameter_number == 0){
		parameter_number++;
		sprintf((char *) pcWriteBuffer,"Running Motor to Zero");
		motor_command.direction = 'z';
		motor_command.run_time = 0;
		xQueueSendToBack( motorque,
		(const void *) &motor_command,
		0);
		return_value = pdTRUE;
	} else {
		M_position = 0;
		/* No more parameters were found.  Make sure the write buffer does
		not contain a valid string. */
		pcWriteBuffer[0] = 0x00;
		parameter_number = 0;
		return_value = pdFALSE;
	}
	
	
	return return_value;
}

/*-----------------------------------------------------------*/

static portBASE_TYPE range_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString){
	//int8_t *parameter_string;
	portBASE_TYPE return_value;
	static portBASE_TYPE parameter_number = 0;
	//static const uint8_t *const running_message = (uint8_t *) "Motor running \r\n";
	//static const uint8_t *const stopping_message = (uint8_t *) "Motor stopping \r\n";
	//static const uint8_t *const que_failed_message = (uint8_t *) "que failed \r\n";
	//static const ticks_per_ms = pdMS_TO_TICKS( 1 );

	motor_que_item motor_command;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	
	if(parameter_number == 0){
		parameter_number++;
		sprintf((char *) pcWriteBuffer,"finding range\r\n");
		motor_command.direction = 'r';
		motor_command.run_time = 0;
		xQueueSendToBack( motorque,
		(const void *) &motor_command,
		0);
		return_value = pdTRUE;
	} else {
		/* No more parameters were found.  Make sure the write buffer does
		not contain a valid string. */
		pcWriteBuffer[0] = 0x00;
		parameter_number = 0;
		return_value = pdFALSE;
	}
	
	
	return return_value;
}

/*-----------------------------------------------------------*/



yytoken_kind_t quicklex(char * input_string, int input_length);

static portBASE_TYPE loop_command(int8_t *pcWriteBuffer,
size_t xWriteBufferLen,
const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;
	
	//static const ticks_per_ms = pdMS_TO_TICKS( 1 );
	
	int yy_status;
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	if (parameter_number == 0) {
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf((char *) pcWriteBuffer, "The parameters were:\r\n");

		/* Next time the function is called the first parameter will be echoed
		back. */
		parameter_number = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		return_value = pdPASS;
	} else {
		/* Obtain the parameter string. */
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter
									(
										pcCommandString,		/* The command string itself. */
										parameter_number,		/* Return the next parameter. */
										&parameter_string_length	/* Store the parameter string length. */
									);

		if (parameter_string != NULL) {
			/* Return the parameter string. */
			memset(pcWriteBuffer, 0x00, xWriteBufferLen);
			sprintf((char *) pcWriteBuffer, "%ld: ", parameter_number);
			strncat((char *) pcWriteBuffer, (const char *) parameter_string, parameter_string_length);
			strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));
			
			yychar = quicklex((char *)parameter_string, parameter_string_length);
			sloppy_print("yychar value: %d\r\n", yychar);
			yy_status = yypush_parse (global_parser);
			sloppy_print("yypush_parse return: %d\r\n", yy_status);
			/* There might be more parameters to return after this one. */
			return_value = pdTRUE;
			parameter_number++;
			
			
		} else {
			/* No more parameters were found.  Make sure the write buffer does
			not contain a valid string. */
			yychar = EOL;
			sloppy_print("yychar value: %d\r\n", yychar);
			yy_status = yypush_parse (global_parser);
			sloppy_print("yypush_parse return: %d\r\n", yy_status);
			
			yychar = YYEOF;
			sloppy_print("yychar value: %d\r\n", yychar);
			yy_status = yypush_parse (global_parser);
			sloppy_print("yypush_parse return: %d\r\n", yy_status);
			
			pcWriteBuffer[0] = 0x00;

			/* No more data to return. */
			return_value = pdFALSE;

			/* Start over the next time this command is executed. */
			parameter_number = 0;
		}
	
	}
	
	return return_value;
}

yytoken_kind_t quicklex(char * input_string, int input_length)
{	//Quick truncated string for matching//
	char quick_buff[20];
	memset(quick_buff, 0x00, 20); 
	strncpy(quick_buff,input_string,input_length);
	
	re_t regex;
	int matchlength;
	regex = re_compile("^[-+]?[0-9]*\.?[0-9]+$"); 
	if(re_matchp(regex, quick_buff, &matchlength) != -1)
		{ yylval.f= atof(quick_buff); return NUMBER;}
	regex = re_compile("^[pPiIdD][kK]$"); 
	if(re_matchp(regex, quick_buff,&matchlength) != -1)	
		{  
			switch(quick_buff[0]){
				case 'p':
				case 'P':
					yylval.xk = F_PK;
					break;
					
				case 'i':
				case 'I':
					yylval.xk = F_IK;
					break;
					
				case 'd':
				case 'D':
					yylval.xk = F_DK;
					break;
				
				default:
					yylval.xk = F_PK;
					break;
			}
			;return XK;}
	regex = re_compile("^[Ll]oop$");
	if(re_matchp(regex, quick_buff,&matchlength) != -1)
		{ return LOOPID;}
	regex = re_compile("^\n$"); 
	if(re_matchp(regex, quick_buff,&matchlength) != -1)
		{ return EOL;}
	regex = re_compile("^on$");
	if(re_matchp(regex, quick_buff,&matchlength) != -1)
	{ return ON;}
	regex = re_compile("^off$");
	if(re_matchp(regex, quick_buff,&matchlength) != -1)
		{ return OFF;}
	regex = re_compile("^[Pp]osition$");
	if(re_matchp(regex, quick_buff,&matchlength) != -1)
		{ return POSITION;}
	regex = re_compile("^[Ss]peed$");	
	if(re_matchp(regex, quick_buff,&matchlength) != -1)
		{ return SPEED;}
	else { 
		sloppy_print("Mystery token *%s*\n", quick_buff); 
	}
	return EOL;	
}

/*-----------------------------------------------------------*/

static portBASE_TYPE printStats_command(int8_t *pcWriteBuffer,
		size_t xWriteBufferLen,
		const int8_t *pcCommandString)
{
	int8_t *parameter_string;
	portBASE_TYPE parameter_string_length, return_value;
	static portBASE_TYPE parameter_number = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandString;
	(void) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	static int no_out;
	if (parameter_number == 0) {
		no_out = 0;
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf((char *) pcWriteBuffer, "The parameters were:\r\n");

		/* Next time the function is called the first parameter will be echoed
		back. */
		parameter_number = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		return_value = pdPASS;
	} else {
		/* Obtain the parameter string. */
		parameter_string = (int8_t *) FreeRTOS_CLIGetParameter
									(
										pcCommandString,		/* The command string itself. */
										parameter_number,		/* Return the next parameter. */
										&parameter_string_length	/* Store the parameter string length. */
									);

		if (parameter_string != NULL) {
			/* Return the parameter string. */
			memset(pcWriteBuffer, 0x00, xWriteBufferLen);
			sprintf((char *) pcWriteBuffer, "%ld: ", parameter_number);
			strncat((char *) pcWriteBuffer, (const char *) parameter_string, parameter_string_length);
			strncat((char *) pcWriteBuffer, "\r\n", strlen("\r\n"));
			no_out = atoi(parameter_string);
			/* There might be more parameters to return after this one. */
			return_value = pdTRUE;
			parameter_number++;
		} else {
			sloppy_print(" %d \r\n" ,no_out);
			printStats(no_out, xTaskGetTickCount());
			/* No more parameters were found.  Make sure the write buffer does
			not contain a valid string. */
			pcWriteBuffer[0] = 0x00;

			/* No more data to return. */
			return_value = pdFALSE;

			/* Start over the next time this command is executed. */
			parameter_number = 0;
		}
	}

	return return_value;
}

/*-----------------------------------------------------------*/