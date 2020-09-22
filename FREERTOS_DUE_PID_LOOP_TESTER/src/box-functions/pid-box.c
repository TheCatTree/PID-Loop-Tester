/*
 * pid_box.c
 *
 * Created: 22/09/2020 11:55:32 p.m.
 *  Author: TCT
 */ 
#include "../mail_box.h"

mail_box_t * box;
bool pidBox_buildPIDbox(){
	box = CreateMailBox(sizeof(pid_update_item), NUMBERBOXES);
	return true;
}
bool pidBox_checkForUpdate(ControlLoop_t id,pid_update_item * pid){
	return takeMail(box, id, 0, pid);
	
}
bool pidBox_updatePID(ControlLoop_t id,pid_update_item * pid){
	return sendMail(box,id,portMAX_DELAY,pid);
	
}