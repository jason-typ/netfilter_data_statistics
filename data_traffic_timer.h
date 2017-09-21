/*********************************************************
* FILE NAME		:	data_traffic_timer.c
* VERSION		:	1.0
* DESCRIPTION	:	Init timer, to zero upload/download speed
*					of a host. If a host has no data traffic
*					in a long period, delete it.
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	13/10/2016
*********************************************************/
#ifndef _DATA_TRAFFIC_TIMER_H
#define _DATA_TRAFFIC_TIMER_H

extern struct timer_list data_traffic_timer;

extern void data_traffic_timer_init(void);

#endif
