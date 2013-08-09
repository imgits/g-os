#ifndef FCNTL_HEADER
#ifndef ADD_HEADER
	#include "asm.h"
	#include "syscall_handler.h"
	#include "general.h"
	#define ADD_HEADER
#endif

extern inline int open(const char *fullpath, int flags) __attribute__((always_inline));

extern inline int open(const char *fullpath, int flags)
{
	unsigned int params[3];

	params[0]=fullpath;
	params[1]=flags;
	SYSCALL(18,params);
	return  params[2];
}

#endif
