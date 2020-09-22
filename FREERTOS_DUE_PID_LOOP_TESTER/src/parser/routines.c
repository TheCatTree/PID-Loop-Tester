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
  switch (xk)
  {
	case F_PK:
		pid[lId].pK = value;
  		break;
	  
	case F_IK:
		pid[lId].iK = value;
		break;
		
	case F_DK:
		pid[lId].dK = value;
		break;
	
	default:
		break;
  }
  
  pid[lId].loop_id  = lId;
  
  pidBox_updatePID(lId, &pid[lId]);
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