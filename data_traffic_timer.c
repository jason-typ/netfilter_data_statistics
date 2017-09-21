/*********************************************************
* FILE NAME		:	data_traffic_timer.c
* VERSION		:	1.0
* DESCRIPTION	:	init timer, to zero upload/download speed
*					of a host. If a host has no data traffic
*					in a long period, delete it.
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	13/10/2016
*********************************************************/
#include <asm/param.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/list.h>
#include "data_traffic_timer.h"
#include "data_traffic_host_entry.h"
#include "data_traffic_tbl_ops.h"

struct timer_list data_traffic_timer;

/********************************************************************
  Function:     data_traffic_timer_function
Description:    zero speed parameters every second, if a hsot doesn't
                have data traffic in a long period, delete it
*********************************************************************/
static void data_traffic_timer_function(unsigned long data)
{
	struct list_head *cursor = NULL;
	struct list_head *tmp = NULL;
	struct host_entry *host = NULL;
	unsigned int time = 0;

	list_for_each_safe(cursor, tmp, g_lru_table) {
		host = list_entry(cursor, struct host_entry, lru_tbl_node);
		time = (jiffies - host->stat.active_time) / HZ;

		host->stat.upload_speed = host->stat.upload_speed_current;
		host->stat.download_speed = host->stat.download_speed_current;

		host->stat.upload_speed_current = 0;
		host->stat.download_speed_current = 0;

		/**
		 * If this host has no data traffic in last 10 mins, delete it.
		 */
		if (time > HOST_EXPIRE_TIME)
			delete_host_entry(host);
	}

	data_traffic_timer.expires = jiffies + HZ;
	add_timer(&data_traffic_timer);
}

/*********************************************************
  Function:     data_traffic_timer_init
  Description:  initialize timer, set the interval 1 second
**********************************************************/
void data_traffic_timer_init(void)
{
	data_traffic_timer.expires = jiffies + HZ;
	data_traffic_timer.data = 0;
	data_traffic_timer.function = data_traffic_timer_function;
	init_timer(&data_traffic_timer);
}
