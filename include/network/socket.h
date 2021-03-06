#ifndef SOCKET_H                
#define SOCKET_H

#include "system.h"
#include "data_types/hashtable.h"
#include "data_types/queue.h"
#include "data_types/primitive_types.h"

#define SOCKET_MAP_SIZE 100
#define TCP_MAP_SIZE 20
#define UDP_MAP_SIZE 20

typedef struct s_socket_desc
{
	t_hashtable* sd_map;
	t_hashtable* tcp_map;
	t_hashtable* udp_map;
	u16 udp_port_indx;
	u16 tcp_port_indx;
	u32 fd;
}
t_socket_desc;

struct s_spinlock_desc;

typedef struct s_socket
{
	struct s_spinlock_desc* lock;
	struct t_process_context* process_context;
	u32 ip;
	u32 port;
	t_queue* udp_rx_queue;	
	u32 type;
	u32 sd;
}
t_socket;

t_socket_desc* socket_desc_init();
void socket_desc_free(t_socket_desc* socket_desc);
int _open_socket(t_socket_desc* socket_desc,int type);
int _bind(t_socket_desc* socket_desc,int sockfd,u32 ip,u32 dst_port);
int _recvfrom(t_socket_desc* socket_desc,int sockfd,unsigned char* src_ip,unsigned char* src_port,void* data,u32 data_len);
int _sendto(t_socket_desc* socket_desc,int sockfd,u32 dst_ip,u16 dst_port,void* data,u32 data_len);
int _close_socket(t_socket_desc* socket_desc,int sockfd);

#endif
