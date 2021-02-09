# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
#define portBASE_TYPE	long
#include "demo-tasks.h"
#include "loopcommands.h"
#include "loopcommands.tab.h"
#include "../mail_box.h"



pid_update_item pid[NUMBERBOXES];

void sloppy_print(const char *s, ...){
	va_list ap;
	va_start(ap, s);
	uint8_t * output_string = (uint8_t *) Stevens_CLIGetOutputBuffer();
	vsprintf((char *) output_string, s, ap);
	va_end(ap);
	uart_cli_output( output_string );
	output_string[0] = 0x00;
	
}

void setUpCommands(cmnd_t * xks_a, int number_loops ){
  for(int i =0 ; i < number_loops; i++){
    cmnd_t x;
    xks_a[i] = x;
    xks_a[i].id = i;
    xks_a[i].flags = 0;
    xks_a[i].pk = 0;
    xks_a[i].ik = 0;
    xks_a[i].dk = 0;
  }
}

void allOn(){
  //printf("ALL on.\n");
  sloppy_print("ALL on.\n");
  const EventBits_t xLoopsOn = (m_update_loop_flag |
								m_position_loop_flag |
								m_speed_loop_flag );
  xEventGroupSetBits(m_control_flags, xLoopsOn);
}
void allOff(){
  //printf("ALL off.\n");
  sloppy_print("ALL off.\n");
  const EventBits_t xLoopsOn = (m_update_loop_flag |
								m_position_loop_flag |
								m_speed_loop_flag );
	xEventGroupClearBits(m_control_flags, xLoopsOn);
}
void updateCommandXk(int lId, xk_flag_t xk, float value){
  //printf("Loop id: %d, xK: %d, Value: %f .", lid , xk ,value);
  sloppy_print("Loop id: %d, xK: %d, Value: %f .", lId , xk ,value);
  
  // Get correct loop type.
  ControlLoop_t loop_name;
  switch (lId)
  {
	 case 0:
		loop_name = CL_position;
		sloppy_print("Name = cl_position");
		break;
		
	case 1:
		loop_name = CL_speed;
		sloppy_print("Name = cl_speed");
		break;
		
	case 2:
		loop_name = CL_update;
		sloppy_print("Name = cl_update");
		break;
		
	  default:
	  loop_name = CL_update;
	  sloppy_print("Name = cl_update (default)");
		break;
  }
  
  pid[loop_name].pK = -1;
  pid[loop_name].iK = -1;
  pid[loop_name].dK = -1;
  
  switch (xk)
  {
	case F_PK:
		pid[loop_name].pK = value;
  		break;
	  
	case F_IK:
		pid[loop_name].iK = value;
		break;
		
	case F_DK:
		pid[loop_name].dK = value;
		break;
	
	default:
		break;
  }
  
  pid[lId].loop_id  = loop_name;
  
  pidBox_updatePID(loop_name, &pid[loop_name]);
}


void
yyerror(char *s)
{
	uint8_t * output_string = (uint8_t *) Stevens_CLIGetOutputBuffer();
	sprintf((char *) output_string, "%s\n ", s);
	uart_cli_output( output_string );
	output_string[0] = 0x00;
}

void updateWantedSpeed( int x) {
	int xx = x;
	uint8_t * output_string = (uint8_t *) Stevens_CLIGetOutputBuffer();
	sprintf((char *) output_string, "Wanted Speed, %d\n ", xx);
	uart_cli_output( output_string );
	output_string[0] = 0x00;
	M_wanted_speed = (float)((float)xx/60); //rps
}

void updateWantedPosition( int x) {
	int xx = x;
	uint8_t * output_string = (uint8_t *) Stevens_CLIGetOutputBuffer();
	sprintf((char *) output_string, "Wanted Position, %d\n ", xx);
	uart_cli_output( output_string );
	output_string[0] = 0x00;
	M_wanted_position = (xx * getRange()/100 );
}