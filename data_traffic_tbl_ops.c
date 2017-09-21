/*****************************************************
 * FILE NAME		:	data_traffic_tbl_ops.c
 * VERSION			:	1.0
 * DESCRIPTION		:	Operations on hash table, lru table and
 *						free table
 *
 * AUTHOR			:	tangyupeng
 * CREATE DATE		:	11/10/2016
 *****************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/spinlock.h>
#include "data_traffic_tbl_ops.h"
#include "data_traffic_host_entry.h"

#define HASH_FUNC(mac_addr) (*(mac_addr + 4) ^ *(mac_addr + 5)) & (MAX_HOST_NUM - 1)

/********************************************************
  Function:     hlist_find_host_by_mac
  Description:  find host in hash table according to
                MAC address
  Input:        mac_addr, MAC address of the host to find
********************************************************/
struct hlist_node *hlist_find_host_by_mac(unsigned char *mac_addr)
{
	struct hlist_node *temp = g_hash_table[HASH_FUNC(mac_addr)].first;
	unsigned char *saved_mac_addr;
	struct host_entry *host;

	if (mac_addr == NULL)
		return NULL;

	while (temp != NULL) {
		host = hlist_entry(temp, struct host_entry, hash_tbl_node);
		saved_mac_addr = host->info.mac_addr;
		if (strncmp(saved_mac_addr, mac_addr, ETH_ALEN) == 0) {
			return temp;
		}
		temp = temp->next;
	}

	return NULL;
}

/***************************************************
  Function:     hlist_add
  Description:  add a new host entry to hash table
  Input:        mac_address, MAC address of the host
                n, host entry hash table head
***************************************************/
static void hlist_add(unsigned char *mac_addr, struct hlist_node *n)
{
	struct hlist_head *head = &g_hash_table[HASH_FUNC(mac_addr)];

	hlist_add_head(n, head);
}

static void hlist_delete(struct hlist_node *n)
{
	hlist_del(n);
}

/**************************************************
  Function:     list_del_last
  Description:  Get the last entry of the specified
                list and delte this entry
  Input:        which table(list) to delete from
**************************************************/
static struct list_head *list_del_last(struct list_head *head)
{
	struct list_head *temp = NULL;

	if (head->prev == head)
		return NULL;
	else {
		temp = head->prev;
		list_del(head->prev);

		return temp;
	}
}

/***************************************************
  Function:     list_add_first
  Description:  add a new entry right after head
  Input:        new, list head to insert
                head, to which table(list) to insert
***************************************************/
static struct list_head *list_add_first(struct list_head *new,
									struct list_head *head)
{
	if (new == NULL)
		return NULL;
	else {
		list_add(new, head);
		return new;
	}
}

/**************************************************************
  Function:     free_last_lru_entry
  Description:  delete the last entry of lru table, also delete
                this entry from hash table, add it to free table
**************************************************************/
static void free_last_lru_entry(void)
{
	struct list_head *old = NULL;
	struct host_entry *host = NULL;

	old = list_del_last(g_lru_table);
	if (old == NULL) {
		printk(KERN_ERR "lru table is empty\n");
		return;
	}

	/* delete this host entry from lru table */
	host = list_entry(old, struct host_entry, lru_tbl_node);
	/* delete this host entry from hash table */
	hlist_delete(&(host->hash_tbl_node));
	/* add this host entry to free table */
	list_add_first(&(host->free_tbl_node), g_free_table);
}

/***************************************************
  Function:     remove_host_entry
  Description:  delete a host entry, including
                1. delete this entry from hash table
                2. delete this entry from lru table
                3. add this endry to free table
  Input:        host, host entry
***************************************************/
void remove_host_entry(struct host_entry *host)
{
	list_del_init(&(host->lru_tbl_node));
	hlist_delete(&(host->hash_tbl_node));
	list_add_first(&(host->free_tbl_node), g_free_table);

}
/*******************************************************
  Function:     add_new_host_entry
  Description:  add a new host entry, including
                delete this entry from free table
                add this entry into hash table
                add this entry into lru table
  Input:        mac_addr, MAC address of the host
                ip_addr, IP address of the host
                access_device_name, through which net
                                    device this host is
                                    connected to
******************************************************/
void add_new_host_entry(unsigned char *mac_addr, unsigned int ip_addr, char *access_device_name)
{
	struct list_head *free_entry = NULL;
	struct host_entry *host = NULL;
	int i;

	/* if free table is empty, free the olded entry */
	if (g_free_table->prev == g_free_table)
		/* free table is empty, no free entry available */
		free_last_lru_entry();

	/* get a free entry */
	free_entry = g_free_table->prev;
	host = list_entry(free_entry, struct host_entry, free_tbl_node);
	/* record the MAC and IP address in host */
	for (i = 0; i < 6; i++)
		host->info.mac_addr[i] = *(mac_addr + i);

	host->info.ip_addr = ip_addr;
	if (access_device_name)
		memcpy(host->info.access_device_name, access_device_name, strlen(access_device_name));

	host->stat.active_time = jiffies;
	host->stat.download_speed = 0;
	host->stat.download_speed_current = 0;
	host->stat.download_total = 0;
	host->stat.upload_speed = 0;
	host->stat.upload_speed_current = 0;
	host->stat.upload_total = 0;

	/* add this host entry into hash table */
	hlist_add(mac_addr, &host->hash_tbl_node);
	/* add this host entry into lru table */
	list_add_first(&host->lru_tbl_node, g_lru_table);
	/* delete the entry from free table */
	list_del(g_free_table->prev);
}
/********************************
  Function:     table_size
  Description:  get the table size
  Input:        which table
*********************************/
int table_size(struct list_head *list)
{
	struct list_head *temp = list->next;
	int i = 0;

	while (temp != list) {
		i++;
		temp = temp->next;
	}

	return i;
}
