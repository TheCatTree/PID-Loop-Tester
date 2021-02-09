/*
 * pwm_box.c
 *
 * Created: 22/09/2020 11:55:12 p.m.
 *  Author: TCT
 */ 
#include "../mail_box.h"

SemaphoreHandle_t got_mail;
mail_box_t * pwm_box;
bool pwmBox_buildPWMBox( void ){
	got_mail = xSemaphoreCreateBinary();
	pwm_que_item temp;
	temp.loop_id = 0;
	pwm_box = CreateMailBox(sizeof(temp), NUMBERBOXES);
	return true;
	
}

bool pwmBox_halt_till_update(pwm_que_item array[NUMBERBOXES]){
	xSemaphoreTake(got_mail, portMAX_DELAY);
	goToBox(pwm_box,portMAX_DELAY);
	noMutexTakeBroadCastMail(pwm_box, CL_position, &array[CL_position]);
	noMutexTakeBroadCastMail(pwm_box, CL_speed, &array[CL_speed]);
	leaveBox(pwm_box);
	return true;
}
bool pwmBox_sendPWMValue(ControlLoop_t id,pwm_que_item * pwm){
	bool out = sendMail(pwm_box,id,portMAX_DELAY,pwm);
	xSemaphoreGive(got_mail);
	return out;
}
bool pwmBox_readPWMValue(ControlLoop_t id,pwm_que_item * pwm){
	bool out = readMail(pwm_box, id, portMAX_DELAY, pwm);
	return out;
}