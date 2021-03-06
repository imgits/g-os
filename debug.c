#include "virtual_memory/vm.h"
#include "memory_manager/general.h"
#include "debug.h"

extern unsigned int free_mem_list[POOL_NUM];

unsigned int proc[100];
//int t_sched_debug[10][10];
int index_2=0;
unsigned int collect_mem=0;
unsigned int collected_mem[1500];
unsigned int collected_mem_index=0;
unsigned int allocated_block=0;

void check_free_mem()
{
	int last_block;
	int i=0;
	unsigned int buddy_mem;
	unsigned int pool_mem;
	buddy_mem=buddy_free_mem(system.buddy_desc);
	pool_mem=kfree_mem();
	//buddy_check_mem_status(system.buddy_desc);
	//a_fixed_size_check_mem_status();

	int kk=0;
	for (i=0;i<collected_mem_index;i++)
	{
		if (collected_mem[i]!=0)
		{	kk++;
			if (kk==10)
			{
				last_block=i;
			}
		}
	}

	check_active_process();
	if (buddy_mem<160700560)
	{
		PRINTK("BUDDY MEMEMORY LEAK!!!");
		panic();
	}
	if (pool_mem<279000)
	{
		PRINTK("POOL MEMEMORY LEAK!!!");
		panic();
	}
	printk("BUDDY MEMORY=%d \n",buddy_mem);
	printk("POOL MEMORY=%d \n",pool_mem);
	//collected_mem_index=0;
	//allocated_block=0;
}

void check_process_context()
{
	t_llist_node* next;
	t_llist_node* sentinel_node;
	struct t_process_context* process_context;
	int index=0;

	while(index<10)
	{
		sentinel_node=ll_sentinel(system.scheduler_desc->scheduler_queue[index]);
		next=ll_first(system.scheduler_desc->scheduler_queue[index]);
		while(next!=sentinel_node)
		{
			process_context=next->val;
			//if (process_context->curr_sched_queue_index>9 || process_context->curr_sched_queue_index<0)
			if (process_context==NULL)
			{
				PRINTK("panic!!!! \n");
			}
			next=next=ll_next(next);
		}
		index++;
	}
}

void check_active_process()
{
	t_llist_node* next;
	t_llist_node* sentinel_node;
	struct t_process_context* process_context;
	int index=0;
	
	index_2=0;


	while(index<10)
	{
		sentinel_node=ll_sentinel(system.scheduler_desc->scheduler_queue[index]);
		next=ll_first(system.scheduler_desc->scheduler_queue[index]);
		while(next!=sentinel_node)
		{
			process_context=next->val;
			proc[index_2]=process_context->pid;
			index_2++;
			next=next=ll_next(next);
		}
		index++;
	}
}

void collect_mem_alloc(unsigned int page_addr)
{
	struct t_process_context* current_process;

	CURRENT_PROCESS_CONTEXT(current_process);

//	if (page_addr==3425300480)
//	{
//		printk("aa\n");
//	}
	collected_mem[collected_mem_index++]=page_addr;
	if (collected_mem_index>1499)
	{
		panic();
	}
	allocated_block++;
}

void collect_mem_free(unsigned int page_addr)
{
	int found=0;
	unsigned int i=0;
	struct t_process_context* current_process;

	CURRENT_PROCESS_CONTEXT(current_process);
	
//	if (page_addr==3425300480)
//	{
//		printk("bb\n");
//	}

	for (i=0;i<collected_mem_index;i++)
	{
		if (collected_mem[i]==page_addr)
		{
			allocated_block--;
			collected_mem[i]=0;
			found=1;
			break;
		}
	}
	if (found==0) 
	{
		found=1;
	}
}



