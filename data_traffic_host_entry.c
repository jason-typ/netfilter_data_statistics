/*********************************************************
* FILE NAME		:	data_traffic_host_entry.c
* VERSION		:	1.0
* DESCRIPTION	:	Operations on host entries. Including:
*					1. add new host
*					2. delete a host
*					3. update a host's status
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	13/10/2016
*********************************************************/
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include "data_traffic_host_entry.h"
#include "data_traffic_tbl_ops.h"
#include "data_traffic_proc.h"

/* host entry space apply, 64 totally */
struct host_entry g_host_pool[MAX_HOST_NUM];

/* hash table defination */
struct hlist_head g_hash_table[MAX_HOST_NUM];

/* free table head defination */
static struct list_head free_table_entity;
struct list_head *g_free_table = &free_table_entity;

/* recently used table head defination */
static struct list_head lru_table_entity;
struct list_head *g_lru_table = &lru_table_entity;

void host_entry_data_init(void)
{
	int i;

	/* init the three heads in each host entry */
	for (i = 0; i < MAX_HOST_NUM; i++) {
		INIT_LIST_HEAD(&g_host_pool[i].free_tbl_node);
		INIT_LIST_HEAD(&g_host_pool[i].lru_tbl_node);
		INIT_HLIST_NODE(&g_host_pool[i].hash_tbl_node);
	}

	/* init hash table */
	for (i = 0; i < MAX_HOST_NUM; i++)
		INIT_HLIST_HEAD(g_hash_table + i);

	/* init lru table */
	INIT_LIST_HEAD(g_lru_table);

	/* init free table, add all the nodes to free table */
	INIT_LIST_HEAD(g_free_table);
	for (i = 0; i < MAX_HOST_NUM; i++)
		list_add_tail(&g_host_pool[i].free_tbl_node, g_free_table);
}

/*********************************************************************
  Function:     add_host_entry
  Description:  add a new host entry, if free table is empty, free one
                entry from lru table.
  Input:        mac_addr:   MAC address of the host
                ip_addr:    IP address of the host
                access_device_name: access device of the host(eth or ath)

  Return:       return 1 if parameter error, NULL input
                return 0 if add a new host entry successfully
                return -1 if this host entry has already been recorded
********************************************************************/
int add_host_entry(unsigned char *mac_addr, unsigned int ip_addr, char *access_device_name)
{
	if (mac_addr == NULL) {
		printk(KERN_ERR "parameter error\n");
		return -1;
	}

	/* find if the MAC address has been already recorded */
	if (hlist_find_host_by_mac(mac_addr) == NULL) {
		/* didn't find the mac address in hash table, need to record */
		add_new_host_entry(mac_addr, ip_addr, access_device_name);

		return 0;
	} else {
		/* This MAC address has already been recorded */
		return 1;
	}
}

/**************************************************************
  Function:     update_host_stat
  Description:  update information and status of a host
  Input:        mac_addr:   MAC address of the host
                ip_addr:    IP address of the host
                skb:        received data, sk_buff
                flag:       inbound or outbound
                access_device_name: through which net device to
                                    access this host
***************************************************************/
void update_host_stat(unsigned char *mac_addr, unsigned int ip_addr,
						struct sk_buff *skb, int flag, char *access_device_name)
{
	struct iphdr *ip_header = ip_hdr(skb);
	struct hlist_node *hash_node = hlist_find_host_by_mac(mac_addr);
	struct host_entry *host = NULL;

	if (hash_node == NULL) {
		/* If this MAC address has not been recorded in hash table, return */
		goto exit;
	}

	host = hlist_entry(hash_node, struct host_entry, hash_tbl_node);
	/* Update the last active time of this host */
	host->stat.active_time = jiffies;
	/* If IP of this host has been changed, update it */
	if (host->info.ip_addr != ip_addr)
		host->info.ip_addr = ip_addr;
	if (strncmp(host->info.access_device_name, access_device_name, strlen(access_device_name)) != 0)
		memcpy(host->info.access_device_name, access_device_name, strlen(access_device_name));

	if (flag == INBOUND) {
		/* Download, update download information */
		host->stat.download_total += ip_header->tot_len + sizeof(struct ethhdr);
		host->stat.download_speed_current += ip_header->tot_len + sizeof(struct ethhdr);
	} else {
		host->stat.upload_total += ip_header->tot_len + sizeof(struct ethhdr);
		host->stat.upload_speed_current += ip_header->tot_len + sizeof(struct ethhdr);
	}

exit:
	return;
}
/********************************************
  Function:     delete_host_entry
  Description:  delete a host entry
  Input:        which host entry to delete
********************************************/
void delete_host_entry(struct host_entry *host)
{
	remove_host_entry(host);
}
