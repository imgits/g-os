#include "drivers/pit/8253.h" 
#include "asm.h"  
#include "idt.h" 
#include "virtual_memory/vm.h"
#include "drivers/pic/8259A.h" 

#define K_STACK 0x1FFFFB

extern index_2;
extern unsigned int proc[100];

void init_pit()
{	
	static struct t_i_desc i_desc;
	out(BINARIY_COUNT_MODE,CONTROL_WORD);
	out(LOW_QUANTUM,COUNTER_0);
	out(HI_QUANTUM,COUNTER_0);
	i_desc.baseLow=((int)&int_handler_pit) & 0xFFFF;
	i_desc.selector=0x8;
	i_desc.flags=0x08e00; //0x0EF00;
	i_desc.baseHi=((int)&int_handler_pit)>>0x10;
	set_idt_entry(0x20,&i_desc);
}

void int_handler_pit()
{	
	int x;
	int is_schedule=0;
//	short ds;
	struct t_process_context* process_context;
	struct t_process_context* sleeping_process;
	struct t_processor_reg processor_reg;
	t_llist_node* next;
	t_llist_node* sentinel;
	t_llist_node* old_node;
	struct t_process_context* next_process;
	unsigned int queue_index;
	unsigned int priority;
	
	SAVE_PROCESSOR_REG
	EOI_TO_MASTER_PIC
	SWITCH_DS_TO_KERNEL_MODE

	system.time+=QUANTUM_DURATION;

	if (system.int_path_count>0)
	{
		goto exit_handler;
	}
	sleeping_process=system.active_console_desc->sleeping_process;
	
	sentinel=ll_sentinel(system.process_info->sleep_wait_queue);
	next=ll_first(system.process_info->sleep_wait_queue);
	next_process=next->val;
	//THIS STUFF MUST BE MOVED INSIDE ASSIGNED SLEEP MANAGER LIKE IO
	while(next!=sentinel)
	{
		next_process->assigned_sleep_time-=QUANTUM_DURATION;
		next_process->sleep_time+=QUANTUM_DURATION;
		
		if (next_process->sleep_time>1000) 	
		{
			next_process->sleep_time=1000;
		}
		else if (next_process->sleep_time<0)
		{
			next_process->sleep_time=0;
		}

		if (next_process->assigned_sleep_time==0)
		{		
			adjust_sched_queue(next->val);			
			next_process->assigned_sleep_time=0;
			next_process->proc_status=RUNNING;
			queue_index=next_process->curr_sched_queue_index;
			ll_append(system.scheduler_desc->scheduler_queue[queue_index],next_process);
			old_node=next;
			next=ll_next(next);
			ll_delete_node(old_node);
		}
		else 
		{
			next=ll_next(next);
		}
		next_process=next->val;
	}

	if (sleeping_process!=NULL && !system.active_console_desc->is_empty)
	{
		_awake(sleeping_process);
		system.active_console_desc->sleeping_process=NULL;
	}
	else
	{	
		process_context=system.process_info->current_process->val;
		process_context->sleep_time-=QUANTUM_DURATION;
		
		if (process_context->sleep_time>1000) 	
		{
			process_context->sleep_time=1000;
		}
		else if (process_context->sleep_time<0)
		{
			process_context->sleep_time=0;
		}

		if (process_context->proc_status==EXITING)
		{
			is_schedule=2;
		}
		else 
		{
			process_context->tick--;
			if (process_context->tick==0) 
			{
				process_context->tick=TICK;
				is_schedule=1;	
			}
		}
	}
	
	static int xxx=1;
	if (xxx==1)
	{	
		xxx=0;
		char* ip_payload;
		t_data_sckt_buf* data_sckt_buf=alloc_sckt(MTU_ETH);
		data_sckt_buf->network_hdr=data_sckt_buf->data+HEADER_ETH;
		enqueue_sckt(system.network_desc->tx_queue,data_sckt_buf);
	
		char data[]="this is my first ip packet!!!!!";
		ip_payload=data_sckt_buf->network_hdr+HEADER_IP4;
		u32 src_ip=IP_FROM_OCT_TO_LONG(172,16,6,25);
		u32 dst_ip=IP_FROM_OCT_TO_LONG(172,16,6,200);
		kmemcpy(ip_payload,data,31);

		char frame_addr[100]={
				0x00, 0x50, 0x56, 0xc0, 0x00, 0x08, 0x00, 0x0c, 0x29, 0x10, 0x64, 0xb3, 0x08, 0x00, 0x45, 0x00,  
				0x00, 0x3e, 0xb5, 0x42, 0x40, 0x00, 0x40, 0x11, 0x20, 0xe7, 0xac, 0x10, 0x06, 0x64, 0xac, 0x10,  
				0x06, 0x01, 0xc2, 0xb7, 0x11, 0x5c, 0x00, 0x2a, 0xd8, 0x06, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69,  
				0x73, 0x20, 0x61, 0x20, 0x66, 0x61, 0x6b, 0x65, 0x20, 0x75, 0x64, 0x70, 0x20, 0x70, 0x61, 0x63,  
				0x6b, 0x65, 0x74, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x0a
			     };			 




		//FAKE UDP
//		send_packet_ip4(data_sckt_buf,src_ip,dst_ip,31,UDP_PROTOCOL);
//		equeue_packet(system.network_desc);
		send_packet_i8254x(system.network_desc->dev,frame_addr,76);
	}

	//FLUSH NETWORK QUEUES BEFORE EXITING
//	equeue_packet(system.network_desc);
//	dequeue_packet(system.network_desc);
exit_handler:;
//	EXIT_INT_HANDLER(is_schedule,processor_reg);

	static struct t_process_context _current_process_context;                                          
	static struct t_process_context _old_process_context;                                              
	static struct t_process_context _new_process_context;	                                            
	static struct t_processor_reg _processor_reg;                                                       
	static unsigned int _action2;                                                               
                                                                                                            
	CLI                                                                        
	_action2=is_schedule;                                                                                   
	_current_process_context=*(struct t_process_context*)system.process_info->current_process->val;                                  
	_old_process_context=_current_process_context;                                                      
	_processor_reg=processor_reg;                             
	if (_action2>0)                                                                                      
	{                                                                                           
		schedule(&_current_process_context,&_processor_reg);	                          
		_new_process_context=*(struct t_process_context*)(system.process_info->current_process->val);
		_processor_reg=_new_process_context.processor_reg;                              
		SWITCH_PAGE_DIR(FROM_VIRT_TO_PHY(((unsigned int) _new_process_context.page_dir)))                                                          
		DO_STACK_FRAME(_processor_reg.esp-8); 
		if (_action2==2)                                                                              
		{                                                                         
			DO_STACK_FRAME(_processor_reg.esp-8); 	                                
//			free_vm_process(_old_process_context.page_dir,INIT_VM_USERSPACE);
			free_vm_process(&_old_process_context);
//			if (_old_process_context.phy_add_space!=NULL)
//			{
//				buddy_free_page(system.buddy_desc,FROM_PHY_TO_VIRT(_old_process_context.phy_add_space));
//				buddy_free_page(system.buddy_desc,FROM_PHY_TO_VIRT(_old_process_context.phy_user_stack));
//			}
			buddy_free_page(system.buddy_desc,FROM_PHY_TO_VIRT(_old_process_context.phy_kernel_stack)); 	                         
		}      
		RESTORE_PROCESSOR_REG                                                                       
		EXIT_SYSCALL_HANDLER                                                                        
	}                                                                                                   
	else                                                                                                
	{   
		DO_STACK_FRAME(_processor_reg.esp-8);                                                                                  
		RESTORE_PROCESSOR_REG                                                                       
		RET_FROM_INT_HANDLER                                                                        
	}
}

