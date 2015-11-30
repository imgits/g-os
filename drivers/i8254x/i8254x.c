#include "drivers/i8254x/i8254x.h"

static void write_i8254x(t_i8254x* i8254x,u32 address,u32 value)
{
	outdw(value,i8254x->mem_base+address);
}

static u32 read_i8254x(t_i8254x* i8254x,u32 address)
{
	return indw(i8254x->mem_base+address);
}

static u16 read_eeprom_i8254x(t_i8254x* i8254x,u8 addr)
{
	u32 tmp;

	write_i8254x(i8254x,REG_EERD, 1 | ((u32)(addr)<< 8));
	while( !((tmp = read_i8254x(i8254x,REG_EERD)) & (1 << 4)));

	return (u16)((tmp >> 16) & 0xFFFF);
}

static void read_mac_i8254x(t_i8254x* i8254x)
{
	u16 tmp;	

	tmp=read_i8254x(i8254x,0);
	tmp=(tmp && 0xf);
	i8254x->mac_addr.lo=read_i8254x(i8254x,0);

	tmp=read_i8254x(i8254x,1);
	tmp=(tmp && 0xf)<<8;
	(i8254x->mac_addr.lo) |= tmp;

	tmp=read_i8254x(i8254x,2);
	tmp=(tmp && 0xf)<<16;
	i8254x->mac_addr.mi |= tmp;

	tmp=read_i8254x(i8254x,3);
	tmp=(tmp && 0xf)<<24;
	i8254x->mac_addr.mi |= tmp;

	tmp=read_i8254x(i8254x,4);
	tmp=(tmp && 0xf);
	i8254x->mac_addr.hi=read_i8254x(i8254x,0);

	tmp=read_i8254x(i8254x,5);
	tmp=(tmp && 0xf)<<8;
	i8254x->mac_addr.hi |= tmp;
}

void int_handler_i8254x(t_i8254x* i8254x);

static rx_init_i8254x(t_i8254x* i8254x)
{
	int i;
	t_rx_desc_i8254x* rx_desc;
	t_data_sckt_buf* data_sckt_buf;

	rx_desc=kmalloc(sizeof(t_rx_desc_i8254x)*NUM_RX_DESC);
	for (i=0;i<NUM_RX_DESC;i++)
	{
		//ALLOC IN ADVANCE BUFFER TO AVOID MEMCPY
		data_sckt_buf=alloc_sckt(MTU_ETH);
		rx_desc[i].hi_addr=data_sckt_buf->head;
		rx_desc[i].low_addr=kmalloc(RX_BUF_SIZE);
		rx_desc[i].status=0;
	}
	i8254x->rx_desc=rx_desc;
	write_i8254x(i8254x,RDBAL_REG,rx_desc);
	write_i8254x(i8254x,RDBAH_REG,0);
	write_i8254x(i8254x,RDLEN_REG,NUM_RX_DESC*16);
	i8254x->rx_cur=0;
	
	write_i8254x(i8254x,RHD_REG,0);
	write_i8254x(i8254x,RDT_REG,NUM_RX_DESC-1);
	write_i8254x(i8254x,RCTRL_REG, (RCTL_EN | RCTL_SBP | RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_8192));
}

static tx_init_i8254x(t_i8254x* i8254x)
{
	int i;
	t_tx_desc_i8254x* tx_desc;

	tx_desc=kmalloc(sizeof(t_tx_desc_i8254x)*NUM_TX_DESC);
	for (i=0;i<NUM_TX_DESC;i++)
	{
		tx_desc[i].hi_addr=0;
		tx_desc[i].low_addr=0;
		tx_desc[i].status=TSTA_DD;
		tx_desc[i].cmd=0;
	}
	i8254x->tx_desc=tx_desc;
	write_i8254x(i8254x,TDBAL_REG,tx_desc);
	write_i8254x(i8254x,TDBAH_REG,0);
	write_i8254x(i8254x,TDLEN_REG,NUM_RX_DESC*16);
	i8254x->tx_cur=0;

	write_i8254x(i8254x,THD_REG,0);
	write_i8254x(i8254x,TDT_REG,NUM_RX_DESC-1);

	write_i8254x(i8254x,TCTRL_REG,(TCTL_EN| TCTL_PSP | (15 << TCTL_CT_SHIFT) | (64 << TCTL_COLD_SHIFT) | TCTL_RTLC));
}

static start_link_i8254x(t_i8254x* i8254x)
{
	u32 current_state;

	current_state=read_i8254x(i8254x,CTRL_REG);
	write_i8254x(i8254x,CTRL_REG,current_state | CTRL_SLU);
}

static reset_multicast_array(t_i8254x* i8254x)
{
	int index;

	for(index = 0; index < 0x80; index++)
	{
		write_i8254x(i8254x,MLTC_TBL_ARRY_REG+index*4,0);
	}
}

t_i8254x* init_8254x()
{
	t_i8254x* i8254x=NULL;
	struct t_i_desc i_desc;

	i8254x=kmalloc(sizeof(t_i8254x));
	i8254x->mem_base=read_pci_config_word(I8254X_BUS,I8254X_SLOT,I8254X_FUNC,I8254X_MEM_BASE);
	i8254x->irq_line=read_pci_config_word(I8254X_BUS,I8254X_SLOT,I8254X_FUNC,I8254X_IRQ_LINE);
	read_mac_i8254x(i8254x);
	start_link_i8254x(i8254x);

	i_desc.baseLow=((int)&int_handler_i8254x) & 0xFFFF;
	i_desc.selector=0x8;
	i_desc.flags=0x08e00;
	i_desc.baseHi=((int)&int_handler_i8254x)>>0x10;
	set_idt_entry(0x20+i8254x->irq_line,&int_handler_i8254x);

	reset_multicast_array(i8254x);
	read_mac_i8254x(i8254x);
	rx_init_i8254x(i8254x);
	tx_init_i8254x(i8254x);
	return i8254x;
}

void free_8254x(t_i8254x* i8254x)
{
	kfree(i8254x);
}

void int_handler_i8254x(t_i8254x* i8254x)
{
	struct t_processor_reg processor_reg;
	u16 cur;
	u32 status;
	t_rx_desc_i8254x* rx_desc;
	char* frame_addr;
	u32 low_addr;
	u32 hi_addr;
	u16 frame_len;
	t_data_sckt_buf* data_sckt_buf;
	t_sckt_buf_desc* sckt_buf_desc;
	u32 crc;

	SAVE_PROCESSOR_REG
	disable_irq_line(i8254x->irq_line);
	DISABLE_PREEMPTION
	EOI_TO_SLAVE_PIC
	EOI_TO_MASTER_PIC
	STI

	status=read_i8254x(i8254x,REG_ICR);
	if (status & ICR_LSC)
	{
		start_link_i8254x(i8254x);
	}
	else if (status & ICR_RXT0)
	{
		sckt_buf_desc=system.network_desc->rx_queue;
		cur=i8254x->rx_cur;
		rx_desc=i8254x->rx_desc;
		while(rx_desc[cur].status & 0x1)
		{
			//i use 32 bit addressing
			low_addr=rx_desc[cur].low_addr;
			hi_addr=rx_desc[cur].hi_addr;
			frame_addr=FROM_PHY_TO_VIRT(low_addr);
			frame_len=rx_desc[cur].length;

			//ALLOC IN ADVANCE BUFFER TO AVOID MEMCPY
			data_sckt_buf=frame_addr-MTU_ETH;
			crc=frame_addr[frame_len-3]+(frame_addr[frame_len-2]<<8)+(frame_addr[frame_len-1]<<16)+(frame_addr[frame_len-0]<<24);
			if (rx_desc[cur].checksum==crc)
			{
				data_sckt_buf=alloc_sckt(frame_len);
				data_sckt_buf->mac_hdr=data_sckt_buf;
				enqueue_sckt(system.network_desc->rx_queue,data_sckt_buf);
				//ALLOC IN ADVANCE BUFFER TO AVOID MEMCPY				
				data_sckt_buf=alloc_sckt(MTU_ETH);
				rx_desc[cur].low_addr=data_sckt_buf+MTU_ETH;
			}
		}
	}
	enable_irq_line(i8254x->irq_line);
	ENABLE_PREEMPTION
	EXIT_INT_HANDLER(0,processor_reg)
}

void send_packet_i8254x(t_i8254x* i8254x,void* frame_addr,u16 frame_len)
{
	u32 phy_frame_addr;
	u16 cur;
	t_tx_desc_i8254x* tx_desc;

	cur=i8254x->tx_cur;
	tx_desc=i8254x->tx_desc;
	phy_frame_addr=FROM_PHY_TO_VIRT(frame_addr);

	tx_desc[cur].low_addr=phy_frame_addr;
	tx_desc[cur].hi_addr=0;
	tx_desc[cur].length=frame_len;
	tx_desc[cur].cso=0;
	tx_desc[cur].cmd=CMD_EOP | CMD_RS | CMD_RPS; 
	tx_desc[cur].status=0;
	tx_desc[cur].css=0;
	tx_desc[cur].special=0;

	i8254x->tx_cur = (cur + 1) % NUM_TX_DESC;
	write_i8254x(i8254x,TDT_REG,i8254x->tx_cur);
	while(!(tx_desc[cur].status & 0xff));
}















