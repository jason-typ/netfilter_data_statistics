/*****************************************************
 * FILE NAME		:	data_traffic_tbl_ops.h
 * VERSION			:	1.0
 * DESCRIPTION		:	Operations on hash table, lru table and
 *						free table
 *
 * AUTHOR			:	tangyupeng
 * CREATE DATE		:	11/10/2016
 *****************************************************/
#ifndef _DATA_TRAFFIC_TBL_OPS_H
#define _DATA_TRAFFIC_TBL_OPS_H

#include "data_traffic_host_entry.h"

extern struct hlist_node *hlist_find_host_by_mac(unsigned char *mac_addr);
extern void add_new_host_entry(unsigned char *mac_addr, unsigned int ip_addr, char *access_device_name);
extern void remove_host_entry(struct host_entry *host);
extern int table_size(struct list_head *list);

#endif
