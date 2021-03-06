#include "asm.h"
#include "idt.h"
#include "virtual_memory/vm.h"  
#include "memory_manager/kmalloc.h" 
#include "idt.h" 
#include "drivers/kbc/8042.h" 
#include "lib/lib.h"

char lowercase_charset[0x80] = {
			[0x00]='\0',			
			[0x01]='\0',
			[0x02]='1',
			[0x03]='2',
			[0x04]='3',
			[0x05]='4',
			[0x06]='5',		
			[0x07]='6',
			[0x08]='7',
			[0x09]='8',
			[0x0a]='9',
			[0x0b]='0',
			[0x0c]='-',
			[0x0d]='=',
			[0x0e]='\b',
			[0x0f]='\0',
			[0x10]='q',
			[0x11]='w',
			[0x12]='e',
			[0x13]='r',
			[0x14]='t',
			[0x15]='y',
			[0x16]='u',
			[0x17]='i',
			[0x18]='o',
			[0x19]='p',
			[0x1a]='[',
			[0x1b]=']',
			[0x1c]='\r',
			[0x1d]='\0',
			[0x1e]='a',
			[0x1f]='s',
			[0x20]='d',
			[0x21]='f',
			[0x22]='g',
			[0x23]='h',
			[0x24]='j',
			[0x25]='k',
			[0x26]='l',
			[0x27]=';',
			[0x28]='"',
			[0x29]='\0',
			[0x2a]='\0', //left shift
			[0x2b]='\\',
			[0x2c]='z',
			[0x2d]='x',
			[0x2e]='c',
			[0x2f]='v',
			[0x30]='b',
			[0x31]='n',
			[0x32]='m',
			[0x33]=',',
			[0x34]='.',
			[0x35]='/',
			[0x36]='\0', //right shift
			[0x37]='\0',
			[0x38]='\0',
			[0x39]=' ',
			[0x3a]='\0',
			[0x3b]='\0',
			[0x3c]='\0',
			[0x3d]='\0',
			[0x3e]='\0',
			[0x3f]='\0',
			[0x40]='\0',
			[0x41]='\0',
			[0x42]='\0',
			[0x43]='\0',
			[0x44]='\0',
			[0x45]='\0',
			[0x46]='\0',
			[0x47]='\0',
			[0x48]='\0',
			[0x49]='\0',
			[0x4a]='\0',
			[0x4b]='\0',
			[0x4c]='\0',
			[0x4d]='\0',
			[0x4e]='\0',
			[0x4f]='\0',
			[0x50]='\0',
			[0x51]='\0',
			[0x52]='\0',
			[0x53]='\b',
			[0x54]='\0',
			[0x55]='\0',
			[0x56]='\0',
			[0x57]='\0',
			[0x58]='\0',
			[0x59]='\0',
			[0x5a]='\0',
			[0x5b]='\0',
			[0x5c]='\0',
			[0x5d]='\0',
			[0x5e]='\0',
			[0x5f]='\0',
			[0x60]='\0',
			[0x61]='\0',
			[0x62]='\0',
			[0x63]='\0',
			[0x64]='\0',
			[0x65]='\0',
			[0x66]='\0',
			[0x67]='\0',
			[0x68]='\0',
			[0x69]='\0',
			[0x6a]='\0',
			[0x6b]='\0',
			[0x6c]='\0',
			[0x6d]='\0',
			[0x6e]='\0',
			[0x6f]='\0',
			[0x70]='\0',
			[0x71]='\0',
			[0x72]='\0',
			[0x73]='\0',
			[0x74]='\0',
			[0x75]='\0',
			[0x76]='\0',
			[0x77]='\0',
			[0x78]='\0',
			[0x79]='\0',
			[0x7a]='\0',
			[0x7b]='\0',
			[0x7c]='\0',
			[0x7d]='\0',
			[0x7e]='\0',
			[0x7f]='\0'
};

char uppercase_charset[0x80] = {
			[0x00]='\0',			
			[0x01]='\0',
			[0x02]='!',
			[0x03]='@',
			[0x04]='#',
			[0x05]='$',
			[0x06]='%',		
			[0x07]='^',
			[0x08]='&',
			[0x09]='*',
			[0x0a]='(',
			[0x0b]=')',
			[0x0c]='_',
			[0x0d]='+',
			[0x0e]='\b',
			[0x0f]='\0',
			[0x10]='Q',
			[0x11]='W',
			[0x12]='E',
			[0x13]='R',
			[0x14]='T',
			[0x15]='Y',
			[0x16]='U',
			[0x17]='I',
			[0x18]='O',
			[0x19]='P',
			[0x1a]='{',
			[0x1b]='}',
			[0x1c]='\r',
			[0x1d]='\0',
			[0x1e]='A',
			[0x1f]='S',
			[0x20]='D',
			[0x21]='F',
			[0x22]='G',
			[0x23]='H',
			[0x24]='J',
			[0x25]='K',
			[0x26]='L',
			[0x27]=':',
			[0x28]='"',
			[0x29]='\0',
			[0x2a]='\0', //left shift
			[0x2b]='|',
			[0x2c]='Z',
			[0x2d]='X',
			[0x2e]='C',
			[0x2f]='V',
			[0x30]='B',
			[0x31]='N',
			[0x32]='M',
			[0x33]='<',
			[0x34]='>',
			[0x35]='?',
			[0x36]='\0', //right shift
			[0x37]='\0',
			[0x38]='\0',
			[0x39]=' ',
			[0x3a]='\0',
			[0x3b]='\0',
			[0x3c]='\0',
			[0x3d]='\0',
			[0x3e]='\0',
			[0x3f]='\0',
			[0x40]='\0',
			[0x41]='\0',
			[0x42]='\0',
			[0x43]='\0',
			[0x44]='\0',
			[0x45]='\0',
			[0x46]='\0',
			[0x47]='\0',
			[0x48]='\0',
			[0x49]='\0',
			[0x4a]='\0',
			[0x4b]='\0',
			[0x4c]='\0',
			[0x4d]='\0',
			[0x4e]='\0',
			[0x4f]='\0',
			[0x50]='\0',
			[0x51]='\0',
			[0x52]='\0',
			[0x53]='\b',
			[0x54]='\0',
			[0x55]='\0',
			[0x56]='\0',
			[0x57]='\0',
			[0x58]='\0',
			[0x59]='\0',
			[0x5a]='\0',
			[0x5b]='\0',
			[0x5c]='\0',
			[0x5d]='\0',
			[0x5e]='\0',
			[0x5f]='\0',
			[0x60]='\0',
			[0x61]='\0',
			[0x62]='\0',
			[0x63]='\0',
			[0x64]='\0',
			[0x65]='\0',
			[0x66]='\0',
			[0x67]='\0',
			[0x68]='\0',
			[0x69]='\0',
			[0x6a]='\0',
			[0x6b]='\0',
			[0x6c]='\0',
			[0x6d]='\0',
			[0x6e]='\0',
			[0x6f]='\0',
			[0x70]='\0',
			[0x71]='\0',
			[0x72]='\0',
			[0x73]='\0',
			[0x74]='\0',
			[0x75]='\0',
			[0x76]='\0',
			[0x77]='\0',
			[0x78]='\0',
			[0x79]='\0',
			[0x7a]='\0',
			[0x7b]='\0',
			[0x7c]='\0',
			[0x7d]='\0',
			[0x7e]='\0',
			[0x7f]='\0'
};

static t_queue* in_buf;
static t_llist* wait_queue; 

void static int_handler_kbc();

void init_kbc()
{	
	int i=0;
	struct t_i_desc i_desc;
	
	//printk("kbc init \n");
	in_buf=new_queue();
	i_desc.baseLow=((int)&int_handler_kbc) & 0xFFFF;
	i_desc.selector=0x8;
	i_desc.flags=0x08e00; //0x0EF00;
	i_desc.baseHi=((int)&int_handler_kbc)>>0x10;
	set_idt_entry(0x21,&i_desc);
	out(0xad,0x64);
	in(0x60);
	out(0xae,0x64);
}

void free_kbc()
{
	kfree(in_buf);
}

void int_handler_kbc()
{
	char scan_code;
	char *char_code;
	unsigned int shift_state=0;
	struct t_processor_reg processor_reg;
	
	SAVE_PROCESSOR_REG
//	CLI
	DISABLE_PREEMPTION
	disable_irq_line(1);
	EOI_TO_MASTER_PIC
	STI	
	
	scan_code=in(0x60);
	switch(scan_code) 
	{
        	case 0x2a: 
        		shift_state = 1; 
        		break;
        	case 0xaa: 
			shift_state = 0;
			break;
        	default:
		// need exclude 56 and 29 because i use ctrl+alt in debug mode 
		if (!(scan_code & 0x80) && scan_code!=29 && scan_code!=56) 
		{
			//printk("key pressed \n");
			char_code=&(shift_state ? uppercase_charset:lowercase_charset)[scan_code];
			enqueue(in_buf,char_code);
			system.active_console_desc->is_empty=0;
		}
           	break;
	}
	enable_irq_line(1);
	ENABLE_PREEMPTION
	EXIT_INT_HANDLER(0,processor_reg)
}

char read_buf()
{
	char *char_code;
	char_code=dequeue(in_buf);
	if (char_code!=NULL) 
	{	
		return *char_code;	
	}
	else system.active_console_desc->is_empty=1;
	return 0;
}






