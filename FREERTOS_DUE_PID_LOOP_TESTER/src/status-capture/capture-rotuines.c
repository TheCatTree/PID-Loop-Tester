/*
 * capture_rotuines.c
 *
 * Created: 21/09/2020 8:09:29 p.m.
 *  Author: TCT
 */ 

#include "motor-move.h"
#include "ring_buffer.h"
#include "..\parser\loopcommands.h"
#include "status-capture.h "

#include "stdint.h"
#include "stdbool.h"

bool buffer_in_use = false;
void bufferGard(void);
void bufferGard(void){
	while (buffer_in_use)
	{
		;
	}
}

void captureStats(float speed, int position, uint32_t ticks){
	bufferGard();
	buffer_in_use = true;
	
	status_capture out = {.speed = speed,.position = position,.ticks = ticks};
	
	buffer_push(out);
	buffer_in_use = false;
}

void printStats(int amount, uint32_t time){
	sloppy_print("amount: *%d* . \r\n", amount);
	bufferGard();
	buffer_in_use = true;
	
	status_capture out;

	sloppy_print("block start *********{ \r\n");
	for (int i =0; i < amount; i++)	
	{
		if(buffer_isEmpty()){
			sloppy_print("empty********* \r\n");
			break;
			}
		out = buffer_read();
		sloppy_print("%d,%d,%f\r\n",out.ticks,out.position,out.speed);
	}
	sloppy_print(" \r\n}********* block end \r\n");
	buffer_in_use = false;
}

void printFullBuffer(uint32_t time){
	bufferGard();
	buffer_in_use = true;
	
	status_capture out ;
	const int overflow = 1002;
	int x = 0;
	sloppy_print("block start *********{ \r\n");
		while (!buffer_isEmpty())
		{
			x++;
			out = buffer_read();
			
			sloppy_print("%d,%d,%f\r\n",out.ticks,out.position,out.speed);
			if(x > overflow){
				sloppy_print(" \r\n}********* Stuck in loop \r\n");
				break;
			}
		}
	sloppy_print(" \r\n}********* block end \r\n");
	buffer_in_use = false;
}

void cleanCaptures(void){
	buffer_drop();
}