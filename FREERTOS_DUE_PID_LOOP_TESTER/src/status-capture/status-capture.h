/*
 * status_capture.h
 *
 * Created: 21/09/2020 8:11:32 p.m.
 *  Author: TCT
 */ 

#ifndef STATUSCAPTURE
#define STATUSCAPTURE

void captureStats(float speed, int position, uint32_t ticks);
void printStats(int amount, uint32_t time);
void cleanCaptures( void );


#endif