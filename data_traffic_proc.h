/******************************************************
* FILE NAME		:	data_traffic_proc.h
* VERSION		:	1.0
* DESCRIPTION	:	output the recorded host information
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	08/10/2016

* HISTORY		:
******************************************************/
#ifndef _DATA_TRAFFIC_PROC_H
#define _DATA_TRAFFIC_PROC_H

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "data_traffic_host_entry.h"

#define PROC_FILE_NAME "statistics"

extern int proc_seq_open(struct inode *inode, struct file *filp);

#endif
