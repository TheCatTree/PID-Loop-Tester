/*
 * pwm_box.c
 *
 * Created: 22/09/2020 11:55:12 p.m.
 *  Author: TCT
 */ 
#include "../mail_box.h"

SemaphoreHandle_t got_mail;
mail_box_t * box;
bool pwmBox_buildPWMBox( void ){
	got_mail = xSemaphoreCreateBinary();
	box = CreateMailBox(sizeof(pwm_que_item), NUMBERBOXES);
	return true;
	
}

bool pwmBox_halt_till_update(pwm_que_item array[NUMBERBOXES]){
	xSemaphoreTake(got_mail, portMAX_DELAY);
	goToBox(box,portMAX_DELAY);
	noMutexTakeBroadCastMail(box, CL_position, &array[CL_position]);
	noMutexTakeBroadCastMail(box, CL_speed, &array[CL_speed]);
	leaveBox(box);
	return true;
}
bool pwmBox_sendPWMValue(ControlLoop_t id,pwm_que_item * pwm){
	bool out = sendMail(box,id,portMAX_DELAY,pwm);
	xSemaphoreGive(got_mail);
	return out;
}
bool pwmBox_readPWMValue(ControlLoop_t id,pwm_que_item * pwm){
	bool out = readMail(box, id, portMAX_DELAY, pwm);
	return out;
}