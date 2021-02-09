/*
 * ring_buffer.h
 *
 * Created: 21/09/2020 8:11:08 p.m.
 *  Author: TCT
 */ 
#include <stdio.h>
#ifndef RINGBUFFER
#define RINGBUFFER

#define BUFFERMAX 3000
#include <stdbool.h>
#include <FreeRTOS.h>

// your declarations (and certain types of definitions) here
void buffer_push(uint32_t data);
uint32_t buffer_pop( void );
uint32_t buffer_read( void );
void buffer_drop( void );
uint32_t buffer_peak( void );
bool buffer_isEmpty( void );

#endif