#include "network/ip4.h"

static u16 checksum(byte* addr,u32 count)
{
 	register u32 sum = 0;

 	while(count > 1)
  	{
    	sum = sum + *((word16 *) addr)++;
    	count = count - 2;
  	}

 	if (count > 0)
	{
		sum = sum + *((byte *) addr);
	}

  	while (sum>>16)
	{
    		sum = (sum & 0xFFFF) + (sum >> 16);
	}
  	return(~sum);
}

int send_packet_ip4(t_data_sckt_buf* data_sckt_buf,u32 src_ip,u32 dest_ip,void* data,u16 data_len,u8 protocol)
{
	u16 packet_len;
	char* ip_row_packet;
	u16 chksum_val;

	if (data_len+HEADR_IP4>MTU_IP4)
	{
		return -1;
	}

	src_mac=from_ip_to_mac(src_ip)
	if (src_mac.hi==0xff)
	{
		return -1;
	}

	dest_ip=from_mac_to_ip(dest_ip)
	if (dest_mac.hi==0xff)
	{
		return -1;
	}
	
	packet_len=data_len+IP4_FIX_HEADER_SIZE;
	
	if (packet_len<=MTU_ETH)
	{
		ip_row_packet=data_sckt_buf->network_hdr;
		data_sckt_buf->mac_hdr=data_sckt_buf->network_hdr-HEADER_IP4;
	
		//PACKET TAKES BIG ENDIAN/NETWORK BYTE ORDER
		ip_row_packet[0]= (1>>3) | 5; 			//VERSION(4) IHL(4)
		ip_row_packet[1]=0;	 			//DSFIELD(6) ECN(2)
		ip_row_packet[2]=LOW_16(packet_len);     	//LOW TOTAL LENGHT(8)
		ip_row_packet[3]=HI_16(packet_len);      	//HI TOTAL LENGHT(8)
	
		ip_row_packet[4]=LOW_16(ip4_desc.packet_id);    //LOW PACKET ID(8)       
		ip_row_packet[5]=HI_16(ip4_desc.packet_id);     //HI PACKET ID(8)	
		ip_row_packet[6]=0				//LOW FLAG AND FRAG OFFSET
		ip_row_packet[7]=0				//HI FLAG AND FRAG OFFSET
	
		ip_row_packet[8]=64;				//TTL(8)
		ip_row_packet[9]=protocol;   			//PROTOCOL(8)
		ip_row_packet[10]=0;             		//LOW HEADER CRC(8)
		ip_row_packet[11]=0;             		//HI HEADER CRC(8)
	
		ip_row_packet[12]=IP_LOW_OCT(src_ip);          	//LOW SRC IP(8)
		ip_row_packet[13]=IP_MID_LFT_OCT(src_ip);	//MID RIGHT  SRC IP(8)
		ip_row_packet[14]=IP_MID_RGT(src_ip); 		//MID LEFT SRC IP(8)
		ip_row_packet[15]=IP_HI_OCT(src_ip);		//HI SRC IP(8)

		ip_row_packet[16]=IP_LOW_OCT(dest_ip);          //LOW DST IP(8)
		ip_row_packet[17]=IP_MID_LFT_OCT(dest_ip);	//MID RIGHT DST IP(8)
		ip_row_packet[18]=IP_MID_RGT(dest_ip);          //MID LEFT IP(8)
		ip_row_packet[19]=IP_HI_OCT(dest_ip);           //HI DST IP(8)

		chksum_val=checksum(ip_row_packet,IP4_FIX_HEADER_SIZE);
		ip_row_packet[10]=LOW_16(chksum_val);
		ip_row_packet[11]=HI_16(chksum_val);

	}
	else
	{
		//make here fragmentation
	}
}

void rcv_packet_ip4(t_data_sckt_buf* data_sckt_buf)
{
	char* ip_row_packet;
	u16 chksum_val;
	u32 src_ip;

	ip_row_packet=data_sckt_buf->network_hdr;
	chksum_val=ip_row_packet[10]+(ip_row_packet[11]<<8);
	src_ip=ip_row_packet[12]+(ip_row_packet[13]<<8)+(ip_row_packet[14]<<8)+(ip_row_packet[15]<<24);
	if ((checksum(ip_row_packet,IP4_FIX_HEADER_SIZE)+chksum_val==0) && src_ip==LOCAL_IP)
	{
		if(ip_row_packet[9]==TCP_PROTOCOL)
		{
			//TCP
		}
		else if(ip_row_packet[9]==UDP_PROTOCOL)
		{
			data_sckt_buf->transport_hdr=data_sckt_buf->transport_hdr-HEADER_UDP;
			rcv_packet_udp(data_sckt_buf);
		}
		else if(ip_row_packet[9]==ICMP_PROTOCOL)
		{

		}
	}
}
