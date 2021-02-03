

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Demo includes. */
#include "partest.h"
#include "demo-tasks.h"
#include "conf_example.h"

/* ASF includes. */
#include "sysclk.h"
#include <asf.h>

/* Motor Include */
#include "mail_box.h"
#include "parser/loopcommands.tab.h"
#include <string.h>
#include "status-capture/status-capture.h"

/* Defines the LED toggled to provide visual feedback that the system is
 * running.  The rate is defined in milliseconds, then converted to RTOS ticks
 * by the portTICK_RATE_MS constant. */
#define mainSOFTWARE_TIMER_LED                  (0)
#define mainSOFTWARE_TIMER_RATE                 (200 / portTICK_RATE_MS)

/* Defines the LED that is turned on if an error is detected. */
#define mainERROR_LED                           (1)

/* A block time of 0 ticks simply means "don't block". */
#define mainDONT_BLOCK                          (0)

/* The priorities at which various tasks will get created. */
#define mainUART_CLI_TASK_PRIORITY              (tskIDLE_PRIORITY + 1)
#define mainUSART_CLI_TASK_PRIORITY             (tskIDLE_PRIORITY + 1)
#define mainCDC_CLI_TASK_PRIORITY               (tskIDLE_PRIORITY + 1)
#define mainUSART_ECHO_TASK_PRIORITY            (tskIDLE_PRIORITY)
#define mainSPI_FLASH_TASK_PRIORITY             (tskIDLE_PRIORITY)
#define mainTWI_EEPROM_TASK_PRIORITY            (tskIDLE_PRIORITY)

/* The stack sizes allocated to the various tasks. */
#define mainUART_CLI_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2)
#define mainUSART_CLI_TASK_STACK_SIZE   (configMINIMAL_STACK_SIZE * 2)
#define mainCDC_CLI_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2)
#define mainUSART_ECHO_TASK_STACK_SIZE  (configMINIMAL_STACK_SIZE)
#define mainSPI_FLASH_TASK_STACK_SIZE   (configMINIMAL_STACK_SIZE * 2)
#define mainTWI_EEPROM_TASK_STACK_SIZE  (configMINIMAL_STACK_SIZE * 2)

/*-----------------------------------------------------------*/

/*
 * Global Error flag setup
 */
 bool Malloc_Fail_flag = false;
 bool StackOverflow_Fail_flag = false;
 
/*
 * Sets up the hardware ready to run this example.
 */
static void prvSetupHardware(void);

/*
 * The callback function used by the software timer.  See the comments at the
 * top of this file.
 */
static void prvLEDTimerCallback(void *pvParameters);

/*
 * FreeRTOS hook (or callback) functions that are defined in this file.
 */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle pxTask,
		signed char *pcTaskName);
void vApplicationTickHook(void);

/*-----------------------------------------------------------*/

/*
 * Interrupt handler for pins on PortD.
 */

//Global Interrupt Count
int d30_interrupt_count = 0;
int d28_interrupt_count = 0;

void pin_edge_handler(const uint32_t id, const uint32_t index);
void pin_edge_handler(const uint32_t id, const uint32_t index)
{
	static bool Alevel;
	static bool Blevel;
	static bool Last_Alevel;
	static bool Last_Blevel;
	

	
	if (id == ID_PIOD)
	{
		if (index == PIO_PD9) {
			A_interrupt++;
			Last_Alevel = Alevel;
			Alevel = pio_get(PIOD, PIO_TYPE_PIO_INPUT, PIO_PD9);
			if (Last_Alevel != Alevel) { //Check that Hall Sensor changed.
			
				Motor_SpeedUpdate = true;
				updateTimeBetweenInterrupts(xTaskGetTickCountFromISR());
				if(Alevel == Blevel) {
					M_position--;
					
				} else if(Alevel != Blevel) {
					M_position++;
				}
			}
			
		} 
		else if(index == PIO_PD3) {
			B_interrupt++;
			Last_Blevel = Blevel;
			Blevel = pio_get(PIOD, PIO_TYPE_PIO_INPUT, PIO_PD3);
			if (Last_Blevel != Blevel){ //Check that Hall Sensor changed.
				Motor_SpeedUpdate = true;
				updateTimeBetweenInterrupts(xTaskGetTickCountFromISR());
				if(Alevel == Blevel){
					M_position++;
				} else if(Alevel != Blevel) {
					M_position--;
				}
			}
		}

	}

}

/*-----------------------------------------------------------*/

/*
 * sets up debug.
 */
 	uint8_t uc_char;
 	uint8_t uc_flag;
	/**
 *  Configure UART for debug message output.
 */

 #define DEBUG_CONF_UART_BAUDRATE  115200
 #define DEBUG_CONF_UART_PARITY  UART_MR_PAR_NO
 #define CONF_UART CONSOLE_UART


/*-----------------------------------------------------------*/
#define PWM_FREQUENCY 1000
#define PERIOD_VALUE  100

static void motor_move_task(void *pvParameters);
static void update_speed(void *pvParameters);
static void capture_stats(void *pvParameters);
static void position_control_loop(void *pvParameters);
static void speed_control_loop(void *pvParameters);
static void pwm_update(void *pvParameters);


int main(void)
{
	xTimerHandle xLEDTimer;

	/* Prepare the hardware to run this demo. */
	prvSetupHardware();

	/* Create the timer that toggles an LED to show that the system is running,
	and that the other tasks are behaving as expected. */
	xLEDTimer = xTimerCreate((const signed char * const) "LED timer",/* A text name, purely to help debugging. */
							mainSOFTWARE_TIMER_RATE,	/* The timer period. */
							pdTRUE,						/* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
							NULL,						/* The timer does not use its ID, so the ID is just set to NULL. */
							prvLEDTimerCallback			/* The function that is called each time the timer expires. */
							);
		motorque = xQueueCreate( 1, sizeof(motor_que_item) ); /* Que for motor communication */						
	/* Create the motor move Tasks */
	BaseType_t motormove_task_build = xTaskCreate(motor_move_task,			/* The task that moves the motor. */
	(const char * const) "MOVE_MOTOR",	/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	configMINIMAL_STACK_SIZE * 2,					/* The size of the stack allocated to the task. */
	NULL ,			/* The parameter is used to pass the already configured UART port into the task. */
	tskIDLE_PRIORITY + 1,						/* The priority allocated to the task. */
	NULL);								/* Used to store the handle to the created task - in this case the handle is not required. */
	configASSERT(motormove_task_build); 
	
	BaseType_t speed_update_task_build = xTaskCreate(update_speed,			/* The task that updates speed value. */
	(const char * const) "UPDATE SPEED",	/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	configMINIMAL_STACK_SIZE,					/* The size of the stack allocated to the task. */
	NULL ,			/* The parameter is used to pass the already configured UART port into the task. */
	tskIDLE_PRIORITY,						/* The priority allocated to the task. */
	NULL);								/* Used to store the handle to the created task - in this case the handle is not required. */
	configASSERT(speed_update_task_build); 
	
	/* Sanity check the timer's creation, then start the timer.  The timer
	will not actually start until the FreeRTOS kernel is started. */
	configASSERT(xLEDTimer);
	xTimerStart(xLEDTimer, mainDONT_BLOCK);
	
	/* loop tasks */
	 xTaskCreate(capture_stats,			/* The task that captures motor stats for analysis. */
	 (const char * const) "CAPTURE STATS",	/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	 configMINIMAL_STACK_SIZE,					/* The size of the stack allocated to the task. */
	 NULL ,			/* The parameter is used to pass the already configured UART port into the task. */
	 tskIDLE_PRIORITY,						/* The priority allocated to the task. */
	 NULL);								/* Used to store the handle to the created task - in this case the handle is not required. */
	 
	 xTaskCreate(pwm_update,			/* The task that updates the PWM value. */
	 (const char * const) "UPDATE PWM",	/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	 configMINIMAL_STACK_SIZE,					/* The size of the stack allocated to the task. */
	 NULL ,			/* The parameter is used to pass the already configured UART port into the task. */
	 tskIDLE_PRIORITY,						/* The priority allocated to the task. */
	 NULL);
	 
	xTaskCreate(position_control_loop,			/* Positional control loop */
	(const char * const) "POSITION CONTROL LOOP",	/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	configMINIMAL_STACK_SIZE,					/* The size of the stack allocated to the task. */
	NULL ,			/* The parameter is used to pass the already configured UART port into the task. */
	tskIDLE_PRIORITY,						/* The priority allocated to the task. */
	NULL);
	
	xTaskCreate(speed_control_loop,			/* Speed control loop */
	(const char * const) "POSITION CONTROL LOOP",	/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	configMINIMAL_STACK_SIZE,					/* The size of the stack allocated to the task. */
	NULL ,			/* The parameter is used to pass the already configured UART port into the task. */
	tskIDLE_PRIORITY,						/* The priority allocated to the task. */
	NULL);
	 
	 
	/* Set up mail box */
	pidBox_buildPIDbox();
	pwmBox_buildPWMBox();
	
	/* Set up event group */
	m_control_flags = xEventGroupCreate( );
	
	
	global_parser = yypstate_new();
	
	/* Create the example tasks as per the configuration settings.
	See the comments at the top of this file. */
	#if (defined confINCLUDE_UART_CLI)
	{
		create_uart_cli_task(BOARD_UART,
				mainUART_CLI_TASK_STACK_SIZE,
				mainUART_CLI_TASK_PRIORITY);
	}
	#endif /* confINCLUDE_USART_CLI */



	#if (defined confINCLUDE_USART_CLI)
	{
		create_usart_cli_task(BOARD_USART_CLI,
				mainUSART_CLI_TASK_STACK_SIZE,
				mainUSART_CLI_TASK_PRIORITY);
	}
	#endif /* confINCLUDE_USART_CLI */

	#if (defined confINCLUDE_CDC_CLI)
	{
		create_usb_cdc_cli_task(mainCDC_CLI_TASK_STACK_SIZE,
				mainCDC_CLI_TASK_PRIORITY);
	}
	#endif /* confINCLUDE_CDC_CLI */

	
	/* Set up code to get pins running */
	//configure_console();

	/************************************************************************/
	/* PWM Setup.                                                                     */
	/************************************************************************/
	pmc_enable_periph_clk(ID_PWM); //Turn PWM on on board;
	
	pwm_channel_disable(PWM, PWM_CHANNEL_2); // Turning off channels to be safe.
	pwm_channel_disable(PWM, PWM_CHANNEL_3);
	
	pwm_clock_t clock_setting = { // Basic Clock set up.
		.ul_clka = PERIOD_VALUE * PWM_FREQUENCY,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_cpu_hz(),
	};
	
	pwm_init(PWM, &clock_setting); 
	
	pwm_channel_2_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_channel_2_instance.ul_period = PERIOD_VALUE;
	pwm_channel_2_instance.ul_duty = PERIOD_VALUE;
	pwm_channel_2_instance.channel = PWM_CHANNEL_2;
	pwm_channel_2_instance.polarity = PWM_HIGH;
	pwm_channel_init(PWM, &pwm_channel_2_instance);
	
	pwm_channel_3_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_channel_3_instance.ul_period = PERIOD_VALUE;
	pwm_channel_3_instance.ul_duty = 0;
	pwm_channel_3_instance.channel = PWM_CHANNEL_3;
	pwm_channel_3_instance.polarity = PWM_HIGH;
	pwm_channel_init(PWM, &pwm_channel_3_instance);
	

	pwm_channel_disable_interrupt(PWM, PWM_CHANNEL_3,0);
	pwm_channel_disable_interrupt(PWM, PWM_CHANNEL_2,0);
	
    NVIC_DisableIRQ(PWM_IRQn);
	NVIC_ClearPendingIRQ(PWM_IRQn);
	NVIC_SetPriority(PWM_IRQn, 0);
	NVIC_EnableIRQ(PWM_IRQn);
	
	NVIC_DisableIRQ(PIOD_IRQn);
	NVIC_ClearPendingIRQ(PIOD_IRQn);
	NVIC_SetPriority(PIOD_IRQn, 10); // Interrupt value 10 is max level FreeRtos can use. I am using because interrupt uses Free rtos time function;
	NVIC_EnableIRQ(PIOD_IRQn);
	pwm_channel_enable(PWM, PWM_CHANNEL_2);
	pwm_channel_enable(PWM, PWM_CHANNEL_3);
	motorStop();
	
	A_interrupt = 0;
	B_interrupt = 0;
	M_position = 0;
	/* Start the RTOS scheduler. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	
	int delay = 1000; 
	//debugMessage();
	//printf("CPU hz: %d \r\n",sysclk_get_cpu_hz());
	bool led = true;
	for (;;) {
		
		/* pin working test code */
		//PWML2 D43 A20 pin 72
		//PWMH3 D41 c9 pin 67
		//digital out D50 c13 pin 95
		//digital out D48 c15 pin 97
		//Hall interrupt 1 d30 d9 pin 22
		//Hall interrupt 2 d28 d3 pin 16
		
		led = !led;
		ioport_set_pin_level(PIO_PC13_IDX, led); 
		delay_ms(delay);

		
		led = !led;	
		ioport_set_pin_level(PIO_PC13_IDX, led); 
		delay_ms(delay);
			//printf("D30: %d, D28: %d \r\n",d30_interrupt_count ,d28_interrupt_count );
		
		led = !led;
		ioport_set_pin_level(PIO_PC13_IDX, led); 
		delay_ms(delay);
	}
}

/*-----------------------------------------------------------*/

static void prvLEDTimerCallback(void *pvParameters)
{
	portBASE_TYPE xStatus = pdPASS;

	/* Just to remove compiler warnings. */
	(void) pvParameters;

	/* Check other tasks. */
	#if (defined confINCLUDE_USART_ECHO_TASKS)
	{ 
		if (are_usart_echo_tasks_still_running() != pdPASS) {
			xStatus = pdFAIL;
		}
	}
	#endif /* confINCLUDE_USART_ECHO_TASKS */

	#if (defined confINCLUDE_SPI_FLASH_TASK)
	{
		if (did_spi_flash_test_pass() != pdPASS) {
			xStatus = pdFAIL;
		}
	}
	#endif /* confINCLUDE_SPI_FLASH_TASK */

	#if (defined confINCLUDE_TWI_EEPROM_TASK)
	{
		if (did_twi_eeprom_test_pass() != pdPASS) {
			xStatus = pdFAIL;
		}
	}
	#endif /* configINCLUDE_TWI_EEPROM_TASK */

	/* If an error has been detected, turn the error LED on. */
	if (xStatus != pdPASS) {
		vParTestSetLED(mainERROR_LED, pdTRUE);
	}

	/* Toggle an LED to show the system is executing. */
	vParTestToggleLED(mainSOFTWARE_TIMER_LED);
}

/*-----------------------------------------------------------*/

static void prvSetupHardware(void)
{
	/* ASF function to setup clocking. */
	sysclk_init();



	/* Atmel library function to setup for the evaluation kit being used. */
	board_init();
	
	/* Ensure all priority bits are assigned as preemption priority bits. */
	NVIC_SetPriorityGrouping(0);
	/* PWM PINS */

	gpio_configure_pin(PIO_PC9_IDX, PIO_PERIPH_B);
	gpio_configure_pin(PIO_PA20_IDX, PIO_PERIPH_B);
	/* digital pins*/
	ioport_set_pin_dir(PIO_PC13_IDX, IOPORT_DIR_OUTPUT); 
	ioport_set_pin_dir(PIO_PC15_IDX, IOPORT_DIR_OUTPUT); 
	ioport_set_pin_level(PIO_PC13_IDX, false); 
	ioport_set_pin_level(PIO_PC15_IDX, false); 
	/*Interrupt pins with pull up*/
	pmc_enable_periph_clk(ID_PIOD); 
	pio_set_input(PIOD, PIO_PD9, PIO_INPUT); 
	pio_set_input(PIOD, PIO_PD3, PIO_INPUT);
	pio_handler_set(PIOD, ID_PIOD, PIO_PD9, PIO_IT_EDGE, pin_edge_handler); 
	pio_handler_set(PIOD, ID_PIOD, PIO_PD3, PIO_IT_EDGE, pin_edge_handler);
	pio_enable_interrupt(PIOD, PIO_PD9); 
	pio_enable_interrupt(PIOD, PIO_PD3);   

	/* Perform any initialization required by the partest LED IO functions. */
	vParTestInitialise();
}

/*-----------------------------------------------------------*/

static void motor_move_task(void *pvParameters){
	static const uint8_t *const running_message = (uint8_t *) "Motor running \r\n";
	static const uint8_t *const stopping_message = (uint8_t *) "Motor stopping \r\n";
	static const uint8_t *const que_failed_message = (uint8_t *) "Queue failed \r\n";
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint8_t *output_string;
	output_string = (uint8_t *) Stevens_CLIGetOutputBuffer();
	bool led = false;
	BaseType_t xStatus;
	motor_que_item recived;
	int runtime;
	int count = 0;
	int last_position;
	 /* As per most tasks, this task is implemented in an infinite loop. */
	 for( ;; )
	 {
		 xStatus = xQueueReceive( motorque, &recived,  portMAX_DELAY );
		 if( xStatus == pdPASS )
		 {
		 /* Data was successfully received from the queue, print out the received
		 value. */
			led = !led;
		     ioport_set_pin_level(PIO_PC15_IDX, true);
		 	 //strcpy((char *) output_string, (char *) running_message);
			 sprintf((char *) output_string, "Que recived, Time: %d ms , Direction: %c\r\n",recived.run_time,recived.direction);
		 	 uart_cli_output( output_string );
			 output_string[0] = 0x00;
			 runtime = recived.run_time;
			last_position = M_position;
			count = 0;
			 if (recived.direction == 'f' || (recived.direction == 'b' || recived.direction == 's'))
			 {
				if (recived.direction == 'f')
				{
					strcpy((char *) output_string, (char *) running_message);
					motorFoward();
				}
				else if(recived.direction == 'b')
				{
					strcpy((char *) output_string, (char *) running_message);
					motorBackward();
				}
				else if(recived.direction == 's')
				{
					strcpy((char *) output_string, (char *) running_message);
					motorStop();
				}
				uart_cli_output( output_string );
				xLastWakeTime = xTaskGetTickCount();
				for (int i=0; i < runtime; i++ )
				{
					vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1 ) );
				}
				 			 
				motorStop();
				strcpy((char *) output_string, (char *) stopping_message);
				uart_cli_output( output_string );
				xStatus = pdFAIL;
			 }
			 if (recived.direction == 'z')
			 {
				motorBackward();
				strcpy((char *) output_string, (char *) running_message);
				uart_cli_output( output_string );
				xLastWakeTime = xTaskGetTickCount();
				last_position = M_position;
				count = 0;
				for (int i=0; i < 100; i++ )
				{
					vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 100 ) );
					if ((last_position <= M_position + 3)&&(last_position >= M_position - 3))
					{
						count++;
						if (count >= 10)
						{
							break;
						}
						
					}
					else
					{
						count = 0;
					}
					last_position = M_position;
				}
				
				motorStop();
				strcpy((char *) output_string, (char *) stopping_message);
				uart_cli_output( output_string );
				M_position = 0;
				sprintf((char *) output_string, "Motor Zeroed. \r\n");
				uart_cli_output( output_string );
				xStatus = pdFAIL;			 
			 }
			 if (recived.direction == 'r')
			 {
				sprintf((char *) output_string, "Entered Range Finder \r\n");
				uart_cli_output( output_string );
				motorBackward();
				sprintf((char *) output_string, "Going Backward \r\n");
				uart_cli_output( output_string );
				xLastWakeTime = xTaskGetTickCount();
				last_position = M_position;
				count = 0;
				for (int i=0; i < 100; i++ )
				{
					sprintf((char *) output_string, "Loop No: %d \r\n",i);
					uart_cli_output( output_string );
					vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 100 ) );
					if ((last_position <= M_position + 3)&&(last_position >= M_position - 3))
					{
						count++;
						if (count >= 10)
						{
							sprintf((char *) output_string, "Count Brake \r\n");
							uart_cli_output( output_string );
							break;
						}
						
					} 
					else
					{
						count = 0;
					}
					last_position = M_position;
				}
				
				motorStop();
				strcpy((char *) output_string, (char *) stopping_message);
				uart_cli_output( output_string );
				M_position = 0;
				motorFoward();
				sprintf((char *) output_string, "Going Forward \r\n");
				uart_cli_output( output_string );
				xLastWakeTime = xTaskGetTickCount();
				last_position = M_position;
				count = 0;
				for (int i=0; i < 100; i++ )
				{
					sprintf((char *) output_string, "Loop No: %d \r\n",i);
					uart_cli_output( output_string );
					vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 100 ) );
					if ((last_position <= M_position + 3)&&(last_position >= M_position - 3))
					{
						count++;
						if (count >= 10)
						{
							sprintf((char *) output_string, "Count Brake \r\n");
							uart_cli_output( output_string );
							break;
						}
										
					}
					else
					{
						count = 0;
					}
					last_position = M_position;
				}
								
				motorStop();
				strcpy((char *) output_string, (char *) stopping_message);
				uart_cli_output( output_string );
				
				updateRange(M_position);
				sprintf((char *) output_string, "Range: %d \r\n",getRange());
				uart_cli_output( output_string );

				xStatus = pdFAIL;				
			 }

		 }
		 else
		 {
		 /* Data was not received from the queue even after waiting for Max Time.
		 This must be an error as the sending tasks are free running and will be
		 continuously writing to the queue. */
		 led = !led;
			ioport_set_pin_level(PIO_PC15_IDX, false);	
		 	strcpy((char *) output_string, (char *) que_failed_message);
		 	uart_cli_output( output_string );
		 }		 output_string[0] = 0x00;
	 }
}

static void update_speed(void *pvParameters){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	TickType_t xLastupDateTime = xTaskGetTickCount();
	static const int ticks_per_ms = pdMS_TO_TICKS( 1 );
	 for( ;; )
	 {
	 
		TickType_t x = getAverageOfTimeBetweenInterrupts();		if (!Motor_SpeedUpdate)
		{	TickType_t y = xTaskGetTickCount() - xLastupDateTime;
			if (y > pdMS_TO_TICKS( 10000 ))
			{
				y = pdMS_TO_TICKS( 10000 );
			}
			x = (x + y);
		}		else{			xLastupDateTime = xTaskGetTickCount();		}		Motor_SpeedUpdate = false;				updateSpeed(1000/(x / ticks_per_ms));		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 10 ) );
	 }
}

static void pwm_update(void *pvParameters){
	
	pwm_que_item pwm_bag[NUMBERBOXES];
	pwm_que_item pwm_using;
	// Any loops not turned on will start with PWMMAX
	for (int i = 0; i < NUMBERBOXES; i++)
	{
		pwm_bag[i].pwm = PWMMAX;
	}
	for( ;; )
	{
		//wait until turned on
		xEventGroupWaitBits( m_control_flags,
		m_update_loop_flag,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY );
		
		//Pwm values available
		pwmBox_halt_till_update(pwm_bag);
		
		// chose smallest one 
		int out = CL_update;
		pwm_bag[out].pwm = PWMMAX;
		for (int i = 0; i < NUMBERBOXES; i++)
		{
			if(i !=CL_update ){
				if (pwm_bag[i].created_time != 0 && pwm_bag[i].received_time == 0 )
				{
					pwm_bag[i].received_time = xTaskGetTickCount();
					
				}
				if (pwm_bag[i].pwm <= pwm_bag[out].pwm)
				{
					out = i;
				}
			}
			
			
		}

		pwm_using = pwm_bag[out];
		// update pwm 
		setDutyCycle(pwm_using.pwm);
		pwmBox_sendPWMValue(CL_update, &pwm_using);
		

	}
}

static void position_control_loop(void *pvParameters){
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	TickType_t loop_time = xTaskGetTickCount();
	TickType_t last_loop_time = xTaskGetTickCount();
	
	position_loop_pK = 2;
	position_loop_iK = 0.2;
	position_loop_dK = 0.02;

	float slope = 0;
	int delta = 0;
	int error = 0;
	
	int snapshot_position = 0;
	int last_position = 0;
	
	pid_update_item pid_update;
	
	pwm_que_item pwm_out;
	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 10 ) );
		//wait until turned on
		xEventGroupWaitBits( m_control_flags,
		m_position_loop_flag,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY );
				
		// test for new PID parameters
		BaseType_t mail = pidBox_checkForUpdate(CL_position,&pid_update);		if (mail == pdPASS)
		{
			if (pid_update.loop_id == CL_position)
			{
				position_loop_pK = pid_update.pK;
				position_loop_iK = pid_update.iK;
				position_loop_dK = pid_update.dK;
			}
		}
						
		//position read & //take time
		last_loop_time = loop_time;
		last_position = snapshot_position;
		loop_time = xTaskGetTickCount();
		snapshot_position = M_position;
		
		//calculate delta for wanted position
		delta = M_wanted_position - snapshot_position;
		
		// calculate error 
		error += delta;
		//calculate change in position
		int change = snapshot_position - last_position;
		//calculate rate of change
		slope = change / (loop_time - last_loop_time);
		//add and round to int
		float f_pwm = position_loop_pK * delta + position_loop_iK * error + position_loop_dK * slope;
		int i_pwm = (int)(f_pwm + 0.5);
		//send
		pwm_out.pwm = abs(i_pwm);
		pwm_out.loop_id = CL_position;
		pwm_out.created_time = xTaskGetTickCount();
		pwm_out.created_time = 0;
		pwmBox_sendPWMValue(CL_position, &pwm_out);
		
		if (((M_wanted_position <= M_position + 3)&&(M_wanted_position >= M_position - 3)))
		{
			motorStop();
		}else{
            if (getDirection()=='f')  {//If motor is moving forward
	            if (delta <=-1)//But motor position is > wanted position causing -PWM
	            {motorBackward();}//Reverse motor
            }
            else if (getDirection()=='b'){//If motor is moving backwards
	            if (delta >=0)//But Wanted position is > Motor position causing +pwm
	            {motorFoward();}//Reverse motor
            }else{motorFoward();}
		}
	}
}

static void speed_control_loop(void *pvParameters){
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	TickType_t loop_time = xTaskGetTickCount();
	TickType_t last_loop_time = xTaskGetTickCount();

	speed_loop_pK = 2;
	speed_loop_iK = 0.2;
	speed_loop_dK = 0.02;

	float slope = 0;
	int delta = 0;
	int error = 0;
	
	float snapshot_speed = 0;
	float last_speed = 0;
	
	pid_update_item pid_update;
	
	pwm_que_item pwm_out;
	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 10 ) );
		//wait until turned on
		xEventGroupWaitBits( m_control_flags,
		m_speed_loop_flag,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY );
		
		// test for new PID parameters
		BaseType_t mail = pidBox_checkForUpdate( CL_speed, &pid_update);		if (mail == pdPASS)
		{
			if (pid_update.loop_id == CL_speed)
			{
				speed_loop_pK = pid_update.pK;
				speed_loop_iK = pid_update.iK;
				speed_loop_dK = pid_update.dK;
			}
		}
		
		//speed read & //take time
		last_loop_time = loop_time;
		last_speed = snapshot_speed;
		loop_time = xTaskGetTickCount();
		snapshot_speed = GetSpeed();
		
		//calculate delta for wanted speed
		delta = M_wanted_speed - snapshot_speed;
		
		// calculate error
		error += delta;
		//calculate change in speed
		int change = snapshot_speed - last_speed;
		//calculate rate of change
		slope = change / (loop_time - last_loop_time);
		//add and round to int
		float f_pwm = abs(speed_loop_pK * delta + speed_loop_iK * error + speed_loop_dK * slope);
		int i_pwm = (int)(f_pwm + 0.5);
		//send
		pwm_out.pwm = i_pwm;
		pwm_out.loop_id = CL_speed;
		pwm_out.created_time = xTaskGetTickCount();
		pwm_out.created_time = 0;
		pwmBox_sendPWMValue(CL_speed, &pwm_out);
		
	}
}

static void capture_stats(void *pvParameters){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		float speed = GetSpeed();
		int ticks = xTaskGetTickCount();
		
		captureStats(speed, M_position, ticks);
		
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 10 ) );
	}
}