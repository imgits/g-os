#define MTU_IP4 65536
#define MTU_ETH 1526
#define HEADER_IP4 20
#define HEADER_ETH 14
#define HEADER_UDP
#define HEADER_TCP
#define SOCKET_BUFFER_SIZE 2000
#define TCP_PROTOCOL 	4
#define UDP_PROTOCOL 	6
#define ICMP_PROTOCOL	1
#define LOCAL_IP	0xC0A8010A
#define LOCAL_NETMASK   0xFFFFFF00   

#define LOW_32(data)		(data_len & 0xFFFF)
#define HI_32(data)		((data>>16) & 0xFFFF)
#define LOW_16(data) 		(data_len & 0xFF)
#define HI_16(data)  		((data>>8) & 0xFF)
#define IP_LOW_OCT(data)	(data & 0xFF)
#define IP_MID_RGT_OCT(data)	(data>>8 && 0xFF)
#define IP_MID_LFT_OCT(data)	(data>>16 && 0xFF)
#define IP_HI_OCT(data)		(data>>24 && 0xFF)

typedef struct s_network_desc
{
	t_sckt_buf_desc* rx_queue;
	t_sckt_buf_desc* tx_queue;
	//WOULD BE BETTER TO ABSTRACT WITH A DEVICE
	t_i8254x* dev;

}
t_network_desc;

t_network_desc* network_init();
void network_free(t_network_desc* network_desc);


