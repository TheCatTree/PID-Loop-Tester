/*
 * pid_box.c
 *
 * Created: 22/09/2020 11:55:32 p.m.
 *  Author: TCT
 */ 
#include "../mail_box.h"

mail_box_t * pid_box;
bool pidBox_buildPIDbox(){
	pid_update_item temp;
	temp.dK = 9;
	temp.iK = 9;
	temp.pK = 9;
	temp.loop_id = CL_update;
	temp.read = false;
	pid_box = CreateMailBox(sizeof(temp), NUMBERBOXES);
	return true;
}
bool pidBox_checkForUpdate(ControlLoop_t id,pid_update_item * pid){
	return takeMail(pid_box, id, 0, pid);
	
}
bool pidBox_updatePID(ControlLoop_t id,pid_update_item * pid){
	return sendMail(pid_box,id,portMAX_DELAY,pid);
	
}