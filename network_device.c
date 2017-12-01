/* This is a file in construction */

#include<linux/init.h>
#include<linux/module.h>
#include<linux/netdevice.h>
#include<linux/etherdevice.h>
#include<linux/fcdevice.h>  //fiber channel
#include<linux/fddidevice.h> //fddidevice
#include<linux/trdevice.h>//token ring


struct net_device *ether_dev[2];

struct net_device *alloc_netdev(int sizeof_priv,
								const char *name,
								void (*setup)(struct net_device*));
								

ether_dev[0]=alloc_netdev(sizeof (struct ether_priv), "ether0",ether_init);
ether_dev[1]=alloc_netdev(sizeof(struct ether_priv),"ether1".ether_init);

if (ether_dev[0]==NULL ||ether_dev[1]++NULL) {
	return 0;
}

struct net_device *alloc_etherdev(int sizeof_priv);
					ether_setup
					
for (i=0;i<2;i++) {
  if(result=register_netdev(ether_dev[i])) {
	  return 0;
  }
}

ether_setup(dev);
dev->open=ether_open;
dev->stop=ether_release;
dev->set_config=ether_config;
dev->hard_start_xmit=ether_tx;
dev->do_ioctl=ether_ioctl;
dev->get_stats=ether_stats;
dev->rebuild_header=ether_header;
dev->hard_header=ether_header;
dev->tx_timeout=ether_tx_timeout;
dev->watchdog_timeo=timeout;
dev->flags |=IFF_NOARP;
dev->features |=NETIF_F_NO_CSUM;
dev->hard_header_cache=NULL;

struct ether_priv * priv=netdev_priv(dev);
memset(priv,0, sizeof(struct ether_priv));
spin_lock_init(&priv->lock);
ether_rx_ints(dev,1);

struct ether_priv {
	struct net_device_stats stats;
	int status;
	struct ether_packet *ppool;
	struct ether_packet *rx_queue;
	int rx_int_enabled;
	int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	spinlock_t lock;	
};


struct net_device {
	
	char name[IFNAMSIZ];
	unsigned long state;
	struct net_device * next;
	int (*init)(struct net_device *dev);
	unsigned long rmem_end;
	unsigned long rmem_start;
	unsigned long mem_end;
	unsigned long mem_start;
	unsigned long base_addr;
	unsigned char irq;
	unsigned char if_port;
	unsigned char dma;
	unsigned short hard_header_len;
	unsigned int mtu;
	unsigned long tx_queue_len;
	unsigned short type;
	unsigned char addr_len;
	unsigned char broadcast[MAX_ADDR_LEN];
	unsigned char dev_addr[MAX_ADDR_LEN];
	unsigned short flags;
//IFF_UP,IFF_BROADCAST,IFF_LOOPBACK,IFF_DEBUG
//IFF_POINTOPOINT, IFF_NOARP, IFF_PROMISC
//IFF_MULTICAST, IFF_ALLMULTI
//IFF_MASTER,IFF_SLAVE
	int features;
	
};


void ether_cleanup() {
	int i;
	for (i=0;i<2;i++) {
		if (ether_dev[i])){
			unregister_netdev(ether_dev[i]);
			ether_teardown_pool(ether_dev[i]);
			free_netdev(ether_dev[i]);
		}		
	}
	return;
}


void ltalk_setup(struct net_device *dev);//LocalTalk 
void fc_setup(struct net_device *dev);//fiber channel
void fddi_setup(struct net_device *dev);//Fiber distributed data interface
void hippi_setup(struct net_device *dev);
//high performance parallel interface high speed interconnect driver
void tr_setup(struct net_device *dev);
//token ring network interface















