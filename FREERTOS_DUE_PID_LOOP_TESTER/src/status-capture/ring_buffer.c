/*
 * ring_buffer.c
 *
 * Created: 21/09/2020 8:10:05 p.m.
 *  Author: TCT
 */ 

#include "ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

uint32_t bufferStorage[BUFFERMAX];
typedef struct circular_buf_t circular_buf_t;
typedef circular_buf_t* cbuf_handle_t;

struct circular_buf_t {
	uint32_t * buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	bool full;
};

circular_buf_t buffer_object = {.buffer = bufferStorage, .head = 0, .tail = 0 ,.max = BUFFERMAX, .full = false};
cbuf_handle_t buffer = &buffer_object;

/// Pass in a storage buffer and size
/// Returns a circular buffer handle
//cbuf_handle_t circular_buf_init(uint32_t* buffer, size_t size);

/// Free a circular buffer structure.
/// Does not free data buffer; owner is responsible for that
//void circular_buf_free(cbuf_handle_t cbuf);

/// Reset the circular buffer to empty, head == tail
void circular_buf_reset(cbuf_handle_t cbuf);

/// Put version 1 continues to add data if the buffer is full
/// Old data is overwritten
void circular_buf_put(cbuf_handle_t cbuf, uint32_t data);

/// Put Version 2 rejects new data if the buffer is full
/// Returns 0 on success, -1 if buffer is full
int circular_buf_put2(cbuf_handle_t cbuf, uint32_t data);

/// Retrieve a value from the buffer
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_get(cbuf_handle_t cbuf, uint32_t * data);

/// Returns true if the buffer is empty
bool circular_buf_empty(cbuf_handle_t cbuf);

/// Returns true if the buffer is full
bool circular_buf_full(cbuf_handle_t cbuf);

/// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t cbuf);

/// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t cbuf);

/*cbuf_handle_t circular_buf_init(uint32_t* buffer, size_t size)
{
	assert(buffer && size);

	cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
	assert(cbuf);

	cbuf->buffer = buffer;
	cbuf->max = size;
	circular_buf_reset(cbuf);

	assert(circular_buf_empty(cbuf));

	return cbuf;
}*/

void circular_buf_reset(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	cbuf->head = 0;
	cbuf->tail = 0;
	cbuf->full = false;
}

/*void circular_buf_free(cbuf_handle_t cbuf)
{
	assert(cbuf);
	free(cbuf);
}*/

bool circular_buf_full(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	return (!cbuf->full && (cbuf->head == cbuf->tail));
}

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	return cbuf->max;
}

size_t circular_buf_size(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	size_t size = cbuf->max;

	if(!cbuf->full)
	{
		if(cbuf->head >= cbuf->tail)
		{
			size = (cbuf->head - cbuf->tail);
		}
		else
		{
			size = (cbuf->max + cbuf->head - cbuf->tail);
		}
	}

	return size;
}

static void advance_pointer(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	if(cbuf->full)
	{
		cbuf->tail = (cbuf->tail + 1) % cbuf->max;
	}

	cbuf->head = (cbuf->head + 1) % cbuf->max;
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_head(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);
	
	
	
	if (cbuf->head == 0)
	{
		cbuf->head = cbuf->max;
	}else {
		cbuf->head = cbuf->head-1;
	}
	
	if (cbuf->full)
	{
		cbuf->tail = (cbuf->tail - 1);
		if (cbuf->tail < 0)
		{
			cbuf->tail = 0;
		}
	}
	// Don't want to say full if it is hitting on empty
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
	configASSERT(cbuf);

	cbuf->full = false;
	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

void circular_buf_put(cbuf_handle_t cbuf, uint32_t data)
{
	configASSERT(cbuf && cbuf->buffer);

	cbuf->buffer[cbuf->head] = data;

	advance_pointer(cbuf);
}

int circular_buf_put2(cbuf_handle_t cbuf, uint32_t data)
{
	int r = -1;

	configASSERT(cbuf && cbuf->buffer);

	if(!circular_buf_full(cbuf))
	{
		cbuf->buffer[cbuf->head] = data;
		advance_pointer(cbuf);
		r = 0;
	}

	return r;
}

int circular_buf_peak(cbuf_handle_t cbuf, uint32_t * data);
int circular_buf_peak(cbuf_handle_t cbuf, uint32_t * data)
{
	configASSERT(cbuf && data && cbuf->buffer);

	int r = -1;

	if(!circular_buf_empty(cbuf))
	{
		*data = cbuf->buffer[cbuf->tail];

		r = 0;
	}

	return r;
}

int circular_buf_get(cbuf_handle_t cbuf, uint32_t * data)
{
	configASSERT(cbuf && data && cbuf->buffer);

	int r = -1;

	if(!circular_buf_empty(cbuf))
	{
		*data = cbuf->buffer[cbuf->tail];
		retreat_pointer(cbuf);

		r = 0;
	}

	return r;
}

int circular_buf_get_reversed(cbuf_handle_t cbuf, uint32_t * data);
int circular_buf_get_reversed(cbuf_handle_t cbuf, uint32_t * data)
{
	configASSERT(cbuf && data && cbuf->buffer);

	int r = -1;

	if(!circular_buf_empty(cbuf))
	{
		*data = cbuf->buffer[cbuf->head];
		retreat_head(cbuf);

		r = 0;
	}
	
	return r;
}

void buffer_push(uint32_t data){
	circular_buf_put(buffer, data);
	};
	
uint32_t buffer_pop(){
	uint32_t out;
	circular_buf_get_reversed(buffer, &out);
	return out;
	};
	
uint32_t buffer_read(){
	uint32_t out;
	circular_buf_get(buffer, &out);
	return out;
};
	
void buffer_drop(){
	circular_buf_reset(buffer);
	};
	
uint32_t buffer_peak(){
	uint32_t out;
	circular_buf_peak(buffer, &out);
	return out;
	};
	
bool buffer_isEmpty(){
	return circular_buf_empty(buffer);
}