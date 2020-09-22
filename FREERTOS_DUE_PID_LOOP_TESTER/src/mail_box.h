/*
 * mail_box.h
 *
 * Created: 17/09/2020 2:06:57 a.m.
 *  Author: TCT
 */ 


#ifndef MAIL_BOX_H_
#define MAIL_BOX_H_
#include "motor-move.h"
QueueHandle_t pwmque;
/*
#define ENUM_MAP(X) \
X(position, 0)    \
X(speed, 1)   \
X(update, 2)

#define X(n, v) n = v,
typedef enum ControlLoop_t {
	ENUM_MAP(X)
} ControlLoop_t;

#undef X

#define MAILBOXADDER(n, v) + 1

#define NUMBERBOXES 0 + ENUM_MAP(MAILBOXADDER);
*/



typedef enum ControlLoop_t {
	CL_position,
	CL_speed,
	CL_update,
} ControlLoop_t;

#define NUMBERBOXES  3
typedef struct
{
	ControlLoop_t loop_id;
	int pwm;
	TickType_t created_time;
	TickType_t received_time;
} pwm_que_item;

typedef struct
{
	float pK;
	float iK;
	float dK;
	int run_time;
	ControlLoop_t loop_id;
	bool read;
} pid_update_item;



typedef struct mail_box {
	void *slots;
	unsigned int sizeofslots;
	SemaphoreHandle_t * slot_flags;
	SemaphoreHandle_t access;
	bool * read_flags;
	bool * direction_flags;
} mail_box_t;	
	

	

mail_box_t * CreateMailBox(int size_of_slot, int number_slots);
bool readMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * return_structure);
bool takeMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * return_structure);
bool giveMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * mail);
bool sendMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * mail);
bool goToBox(mail_box_t * m_box, unsigned int wait_time);
bool leaveBox(mail_box_t * m_box);
bool noMutexTakeBroadCastMail(mail_box_t * m_box, int slot_number, void * return_structure);

bool pwmBox_buildPWMBox( void );
bool pwmBox_halt_till_update(pwm_que_item array[NUMBERBOXES]);
bool pwmBox_sendPWMValue(ControlLoop_t id,pwm_que_item * pwm);
bool pwmBox_readPWMValue(ControlLoop_t id,pwm_que_item * pwm);

bool pidBox_buildPIDbox( void );
bool pidBox_checkForUpdate(ControlLoop_t id,pid_update_item * pid);
bool pidBox_updatePID(ControlLoop_t id,pid_update_item * pid);
#endif /* MAIL_BOX_H_ */