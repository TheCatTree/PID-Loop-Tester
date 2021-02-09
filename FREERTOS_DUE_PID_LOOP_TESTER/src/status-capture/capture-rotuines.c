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
	float speed_bit_ratios = 18000 / 1024;//(2^10 bits)
	float position_bit_ratios = 100 / 256;//(2^8 bits)
	uint32_t speed_mask = 0x000003FF;
	uint32_t tick_mask = 0x00003fff;
	
	uint32_t out = 0;
	uint32_t speed_bytes = (uint32_t)(speed/speed_bit_ratios);
	uint32_t position_bytes = (uint32_t)(((float)M_position/(float)(getRange() *100))/position_bit_ratios);
	uint32_t tick_bytes = ticks;
	position_bytes = position_bytes << 24;
	out = out | position_bytes;
	speed_bytes = speed_bytes & speed_mask;
	speed_bytes = speed_bytes << 14;
	out = out | position_bytes;
	tick_bytes = tick_bytes & tick_mask;
	out = out | tick_bytes;
	//sloppy_print("captured: *%d* bytes: %d %d %d. \r\n", out,tick_bytes,position_bytes,speed_bytes);
	buffer_push(out);
	buffer_in_use = false;
}

void printStats(int amount, uint32_t time){
	sloppy_print("amount: *%d* . \r\n", amount);
	bufferGard();
	buffer_in_use = true;
	float speed_bit_ratios = 18000 / 1024;//(2^10 bits)
	float position_bit_ratios = 100 / 256;//(2^8 bits)
	uint32_t speed_mask = 0x000003FF;
	uint32_t tick_mask = 0x00003fff;
		
	uint32_t out = 0;
	uint32_t last_tick = 0;
	float speed = 0;
	float position = 0;
	uint32_t tick = 0;
	int s_count = 0;
	last_tick = buffer_peak() & tick_mask;
	sloppy_print("block start *********{ \r\n");
	for (int i =0; i < amount; i++)	
	{
		if(buffer_isEmpty()){
			sloppy_print("empty********* \r\n");
			break;
			}
		out = buffer_read();
		tick = out & tick_mask;
		if (tick > last_tick)
		{
			s_count++;
		}
		//tick = time - (0x00003fff - tick) - ( 1024 * s_count);
		
		speed = (float)((out >> 14) & speed_mask)*speed_bit_ratios;
		position = (float)(out >> 24)*position_bit_ratios;
		
		sloppy_print("%d,%f,%f\r\n",tick,position,speed);
	}
	sloppy_print(" \r\n}********* block end \r\n");
	buffer_in_use = false;
}

void cleanCaptures(void){
	buffer_drop();
}