/*
 * mail_box.c
 *
 * Created: 17/09/2020 3:56:01 a.m.
 *  Author: TCT
 */ 
/*
 * mail_box.c
 *
 * Created: 17/09/2020 2:09:28 a.m.
 *  Author: TCT
 */ 

#include "mail_box.h"
#include <stdlib.h>
#include <string.h>

#define  OUT false
#define  IN true
	
mail_box_t * CreateMailBox(int size_of_slot, int number_slots)
{
	mail_box_t * mbox = pvPortMalloc(sizeof(mail_box_t));
	
	mbox->slots = pvPortMalloc(size_of_slot * number_slots);
	mbox->sizeofslots = size_of_slot;
	memset(mbox->slots, 0x00, size_of_slot * number_slots);
	mbox->slot_flags = pvPortMalloc(sizeof(SemaphoreHandle_t) * number_slots);
	
	mbox->access = xSemaphoreCreateMutex();
	mbox->read_flags = pvPortMalloc(sizeof(bool) * number_slots);
	memset(mbox->read_flags, 0x00, sizeof(bool) * number_slots);
	
	mbox->direction_flags = pvPortMalloc(sizeof(bool) * number_slots);
	memset(mbox->direction_flags, 0x00, sizeof(bool) * number_slots);
	
	//build semaphores
	for (int i = 0; i< number_slots; i++)
	{
		mbox->slot_flags[i] = xSemaphoreCreateBinary();
	}
	
	
	return mbox;
}

bool readMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * return_structure){
	bool out = false;
	if( xSemaphoreTake( m_box->access, ( TickType_t ) wait_time ) == pdTRUE )
        {
            /* We were able to obtain the semaphore and can now access the
            shared resource. */

            if (xSemaphoreTake( m_box->slot_flags[slot_number], ( TickType_t ) wait_time ) == pdTRUE)
            {
				memcpy(return_structure, (uint8_t*)m_box->slots + (m_box->sizeofslots * slot_number), m_box->sizeofslots);
				out = true;
				m_box->read_flags[slot_number] = true;
				xSemaphoreGive( m_box->slot_flags[slot_number] );
            }
			else{
				
			}

            /* We have finished accessing the shared resource.  Release the
            semaphore. */
            xSemaphoreGive( m_box->access );
			
        }
       return out;
}

bool takeMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * return_structure){
	bool out = false;
	if( xSemaphoreTake( m_box->access, ( TickType_t ) wait_time ) == pdTRUE )
        {
            /* We were able to obtain the semaphore and can now access the
            shared resource. */

            if (xSemaphoreTake( m_box->slot_flags[slot_number], ( TickType_t ) wait_time ) == pdTRUE)
            {
				memcpy(return_structure, (uint8_t*)m_box->slots + (m_box->sizeofslots * slot_number), m_box->sizeofslots);
				out = true;
				m_box->read_flags[slot_number] = true;
            }
			else{
				
			}

            /* We have finished accessing the shared resource.  Release the
            semaphore. */
            xSemaphoreGive( m_box->access );
			
        }
       return out;
}

bool giveMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * mail){
	bool out = false;
	if( xSemaphoreTake( m_box->access, ( TickType_t ) wait_time ) == pdTRUE )
        {
            /* We were able to obtain the semaphore and can now access the
            shared resource. */

            
			memcpy( (uint8_t*)m_box->slots + (m_box->sizeofslots * slot_number),mail, m_box->sizeofslots);
			out = true;
            m_box->read_flags[slot_number] = false;
			m_box->direction_flags[slot_number] = IN;

			/* Give A Binary Semaphore for slot. */
			xSemaphoreGive( m_box->slot_flags[slot_number]);

            /* We have finished accessing the shared resource.  Release the
            semaphore. */
            xSemaphoreGive( m_box->access );
			
        }
       return out;
}

bool sendMail(mail_box_t * m_box, int slot_number, unsigned int wait_time, void * mail){
	bool out = false;
	if( xSemaphoreTake( m_box->access, ( TickType_t ) wait_time ) == pdTRUE )
        {
            /* We were able to obtain the semaphore and can now access the
            shared resource. */

            
			memcpy( (uint8_t*)m_box->slots + (m_box->sizeofslots * slot_number),mail, m_box->sizeofslots);
			out = true;
            m_box->read_flags[slot_number] = false;
			m_box->direction_flags[slot_number] = OUT;

			/* Give A Binary Semaphore for slot. */
			xSemaphoreGive( m_box->slot_flags[slot_number]);

            /* We have finished accessing the shared resource.  Release the
            semaphore. */
            xSemaphoreGive( m_box->access );
			
        }
       return out;
}


bool goToBox(mail_box_t * m_box, unsigned int wait_time){
	bool out = false;
	if( xSemaphoreTake( m_box->access, ( TickType_t ) wait_time ) == pdTRUE )
		{
			out = true;	
        }
       return out;
}

bool leaveBox(mail_box_t * m_box){
	bool out = false;
	if( xSemaphoreGive( m_box->access) == pdTRUE )
	{
		out = true;
	}
	return out;
}

bool noMutexTakeBroadCastMail(mail_box_t * m_box, int slot_number, void * return_structure){
	bool out = false;
	if (xSemaphoreTake( m_box->slot_flags[slot_number],0) == pdTRUE)
	{
		
		if ((m_box->read_flags[slot_number] == false)&&(m_box->direction_flags[slot_number] == OUT))
		{
			memcpy(return_structure, (uint8_t*)m_box->slots + (m_box->sizeofslots * slot_number), m_box->sizeofslots);
			out = true;
			m_box->read_flags[slot_number] = true;
		}
		
	}
	else{
		
	}
	return out;
}
