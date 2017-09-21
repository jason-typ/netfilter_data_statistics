/******************************************************
* FILE NAME		:	data_traffic_proc.c
* VERSION		:	1.0
* DESCRIPTION	:	output the recorded host information
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	08/10/2016

* HISTORY		:
******************************************************/
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include "data_traffic_proc.h"
#include "data_traffic_host_entry.h"
#include "data_traffic_tbl_ops.h"

static void *proc_seq_start(struct seq_file *m, loff_t *pos);
static void *proc_seq_next(struct seq_file *m, void *v, loff_t *pos);
static void proc_seq_stop(struct seq_file *m, void *v);
static int proc_seq_show(struct seq_file *m, void *v);

/*********************************************
  Function:     dump_ip_addr
  Description:  output the IP address
  Input:        m, to which seq_file to output
                ip_addr, IP address to output
**********************************************/
static void dump_ip_addr(struct seq_file *m, unsigned int ip_addr)
{
	unsigned char *temp = (unsigned char *)(&ip_addr);

    seq_printf(m, "%d.%d.%d.%d\t", temp[0], temp[1], temp[2], temp[3]);
}

/***********************************************
  Function:     dump_mac_addr
  Description:  output the MAC address
  Input:        m, to which seq_file to output
                mac_addr, pointer to MAC address
************************************************/
static void dump_mac_addr(struct seq_file *m, unsigned char *mac_addr)
{
	if (mac_addr == NULL) {
		printk(KERN_ERR "Parameter error, MAC address is NULL\n");
		return;
	}

    seq_printf(m, "%02X:%02X:%02X:%02X:%02X:%02X\t", mac_addr[0],
            mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

/************************************************************
  Function:     dump_host_entry
  Description:  output the host information, including IP/MAC
                address, download/upload speed and total data
                count.
  Input:        m, to which seq_file to output
                host, which host entry to output
************************************************************/
static void dump_host_entry(struct seq_file *m, struct host_entry *host)
{
	if (host == NULL)
		return;

	dump_mac_addr(m, host->info.mac_addr);

	dump_ip_addr(m, host->info.ip_addr);

	seq_printf(m, "%d\t", host->stat.download_speed);
	seq_printf(m, "%d\t", host->stat.upload_speed);
	seq_printf(m, "%llu\t", host->stat.download_total);
	seq_printf(m, "%llu\t", host->stat.upload_total);
	seq_printf(m, "%s\n", host->info.access_device_name);
}

/***********************************************************
  Function:     proc_seq_start
  Description:  iteration functions, return parameter passed
                to "next" function
  Input:        pos, iterator position
************************************************************/
static void *proc_seq_start(struct seq_file *m, loff_t *pos)
{
    if (*pos >= table_size(g_lru_table))
		return NULL;

	return g_lru_table->next;
}

/**************************************************************
  Function:     proc_seq_next
  Description:  iteration function, stops when receives NULL,
                return parameter passed to this function itself
                and "show" function
***************************************************************/
static void *proc_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
    struct list_head *temp_entry = (struct list_head *)v;

    (*pos)++;
	if (temp_entry->next == g_lru_table)
		return NULL;
	else
		return temp_entry->next;
}

static void proc_seq_stop(struct seq_file *m, void *v)
{
	return;
}

/******************************************************
  Function:     proc_seq_show
  Description:  iteration function, output the host info
*******************************************************/
static int proc_seq_show(struct seq_file *m, void *v)
{
	struct list_head *temp = (struct list_head *)v;
	struct host_entry *host =
		list_entry(temp, struct host_entry, lru_tbl_node);

	dump_host_entry(m, host);

	return 0;
}

static const struct seq_operations proc_seq_ops = {
	.start = proc_seq_start,
	.next = proc_seq_next,
	.stop = proc_seq_stop,
	.show = proc_seq_show,
};

int proc_seq_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &proc_seq_ops);
}
