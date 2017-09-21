/**********************************************************************
* FILE NAME		:	data_traffic.c
* VERSION		:	1.0
* DESCRIPTION	:	To achieve host data traffic count.
					Including:
					1. host statistics according to MAC address
					2. host download/upload speed and total data count
*
* AUTHOR		:	tangyupeng
* CREATE DATE	:	08/10/2016
**********************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <net/arp.h>
#include <linux/list.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/byteorder/generic.h>
#include <net/net_namespace.h>
#include "data_traffic_host_entry.h"
#include "data_traffic_tbl_ops.h"
#include "data_traffic_proc.h"
#include "data_traffic_timer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("data traffic statistics module");
MODULE_AUTHOR("Tang Yupeng");
MODULE_ALIAS("netfilter");

extern struct net_device *br_port_dev_get(struct net_device *dev, unsigned char *addr);

/******************************************************************
  Function:     traffic_count
  Description:    HOOK function, track host MAC address, and record
                it's data traffic count.
*******************************************************************/
unsigned int traffic_count(unsigned int hooknum,
							struct sk_buff *skb,
							const struct net_device *in,
							const struct net_device *out,
							int (*okfn)(struct sk_buff *))
{
	struct ethhdr *mac_header = eth_hdr(skb);
	struct iphdr *ip_header = ip_hdr(skb);
	unsigned char *mac_addr = NULL;
	unsigned int ip_addr = 0;
	const unsigned char zero_mac[ETH_ALEN] = {0};
	struct neighbour *neighbour = NULL;
	char *device_name = NULL;
	unsigned int direction = 0;

	if (strncmp(in->name, WAN_DEVICE_NAME, strlen(WAN_DEVICE_NAME)) == 0) {
		/* From wan to lan, download */
		direction = INBOUND;
		ip_addr = htonl(ip_header->daddr);

		neighbour = neigh_lookup(&arp_tbl, &ip_addr, out);
		if (neighbour == NULL) {
			printk(KERN_ERR "Cannot find host according to IP: ");
			return NF_ACCEPT;
        }
		mac_addr = neighbour->ha;

		/* To filter out host whose mac address is all zero */
		if (memcmp(zero_mac, mac_addr, ETH_ALEN) == 0) {
			return NF_DROP;
		}

		device_name = br_port_dev_get(out, mac_addr)->name;
	} else {
		/* From lan, upload */
		direction = OUTBOUND;
		ip_addr = htonl(ip_header->saddr);
		mac_addr = mac_header->h_source;
		device_name = br_port_dev_get(in, mac_addr)->name;
	}

	add_host_entry(mac_addr, ip_addr, device_name);
	update_host_stat(mac_addr, ip_addr, skb, direction, device_name);

	return NF_ACCEPT;
}

/* FORWARD hook, used to record the host address and data traffic count */
static struct nf_hook_ops traffic_count_hook_ops = {
	.hook = traffic_count,
	.owner = THIS_MODULE,
	.hooknum = NF_INET_FORWARD,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};

static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.open = proc_seq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/**************************************************************
  Function:     data_traffic_statistics_init
  Description:  Init data traffic statistics module, including
                1. register HOOK function
                2. register proc file
                3. start timer

  Return:      Return 0 in case of success,
                    -1 in case of failure.
**************************************************************/
static int __init data_traffic_statistics_init(void)
{
    struct proc_dir_entry *parent = NULL;

	printk(KERN_INFO "Init data traffic count module.\n");
	printk(KERN_INFO "Context initial.\n");
	host_entry_data_init();

	printk(KERN_INFO "Register FORWARD hook\n");
	if (nf_register_hook(&traffic_count_hook_ops) < 0) {
		printk(KERN_ERR "Register hook function failed.\n");

		goto exit;
	}

	printk(KERN_INFO "Register proc file system\n");
    parent = init_net.proc_net;
	if (!proc_create(PROC_FILE_NAME, 0, parent, &proc_ops)) {
		printk(KERN_ERR "Create proc file failed.\n");

		goto unregister_forward_hook;
	}

	printk(KERN_INFO "Start timer\n");
	data_traffic_timer_init();
	add_timer(&data_traffic_timer);

	return 0;

unregister_forward_hook:
	nf_unregister_hook(&traffic_count_hook_ops);

exit:
	return -1;
}

/*****************************************************************
  Function:     data_traffic_statistics_exit
  Description:  Clean up data traffic statistics module, including
                1. delete timer
                2. delete proc file
                3. delete HOOK function
*****************************************************************/
static void __exit data_traffic_statistics_exit(void)
{
	printk(KERN_INFO "Prepare to clean up data traffic module.\n");

	printk(KERN_INFO "Delete timer\n");
	del_timer(&data_traffic_timer);

	printk(KERN_INFO "Remove proc entry: data_traffic\n");
	remove_proc_entry(PROC_FILE_NAME, NULL);

	printk(KERN_INFO "Unregister FORWARD hook\n");
	nf_unregister_hook(&traffic_count_hook_ops);
}

module_init(data_traffic_statistics_init);
module_exit(data_traffic_statistics_exit);
