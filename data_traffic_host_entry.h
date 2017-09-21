/******************************************************
* FILE NAME		:	data_traffic_host_entry.h
* VERSION		:	1.0
* DESCRIPTION	:	Operations on host entries. Including:
*					1. add new host
*					2. delete a host
*					3. update a host's status
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	13/10/2016
******************************************************/
#ifndef _DATA_TRAFFIC_HOST_ENTRY_H
#define _DATA_TRAFFIC_HOST_ENTRY_H

#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

#define MAX_HOST_NUM 8
#define HOST_EXPIRE_TIME 120
#define LAN_DEVICE_NAME "br-lan"
#define LAN_DEVICE_DISPLAY_NAME "eth1"
#define WAN_DEVICE_NAME "eth0"
#define WIRELESS_DEVICE_NAME "ath"
#define DEVICE_NAME_LEN 5

#define INBOUND 0
#define OUTBOUND 1

/* record the neighbour's IP and MAC address */
struct host_info {
	unsigned int ip_addr;
	unsigned char mac_addr[ETH_ALEN];
	char access_device_name[DEVICE_NAME_LEN];
};

/* record the neighbour's data statistics */
struct host_stat {
	unsigned int upload_speed;
	unsigned int download_speed;
	unsigned int upload_speed_current;
	unsigned int download_speed_current;
	unsigned long long upload_total;
	unsigned long long download_total;
	unsigned int active_time;
};

/* record each neighbour as a host entry */
struct host_entry {
	struct list_head free_tbl_node;
	struct list_head lru_tbl_node;
	struct hlist_node hash_tbl_node;
	struct host_info info;
	struct host_stat stat;
};

/* host entry space apply, 64 totally */
extern struct host_entry g_host_pool[];

/* hash table defination */
extern struct hlist_head g_hash_table[];

/* free table head defination */
extern struct list_head *g_free_table;

/* recently used table head defination */
extern struct list_head *g_lru_table;

extern struct timer_list data_traffic_timer;

extern void host_entry_data_init(void);

extern void data_traffic_timer_init(void);

extern int add_host_entry(unsigned char *mac_addr, unsigned int ip_addr, char *access_device_name);

extern void update_host_stat(unsigned char *mac_addr,
						unsigned int ip_addr,
						struct sk_buff *skb,
						int flag,
						char *access_device_name);
extern void delete_host_entry(struct host_entry *host);

#endif
