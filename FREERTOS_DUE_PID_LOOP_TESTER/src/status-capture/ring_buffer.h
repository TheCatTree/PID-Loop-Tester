/*
 * ring_buffer.h
 *
 * Created: 21/09/2020 8:11:08 p.m.
 *  Author: TCT
 */ 
#include <stdio.h>
#ifndef RINGBUFFER
#define RINGBUFFER

#define BUFFERMAX 1000

typedef struct {
	float speed;
	int position;
	uint32_t ticks; 
 } status_capture;

#include <stdbool.h>
#include <FreeRTOS.h>

// your declarations (and certain types of definitions) here
void buffer_push(status_capture data);
status_capture buffer_pop( void );
status_capture buffer_read( void );
void buffer_drop( void );
status_capture buffer_peak( void );
bool buffer_isEmpty( void );

#endif