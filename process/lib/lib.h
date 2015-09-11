#ifndef LIB_H                
#define LIB_H

#define NULL 0
#define SYSCALL(syscall_num,params) asm("mov %0,%%eax;mov %1,%%ecx;int $0x80"::"r"(syscall_num),"r"(params):"%eax","%ecx");
#define GET_FROM_STACK(n,var) asm("mov %%ebp,%0;":"=r"(var));var+=n+2;

#define O_CREAT 0b1
#define O_APPEND 0b10
#define O_RDWR 0b100

#define RAND_MAX 4294967294

unsigned int seed;

struct s_stat 
{             
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned int st_size;
	unsigned int st_atime;
	unsigned int st_mtime;
	unsigned int st_ctime;
}
typedef t_stat;

//fcntl.h
int open(const char *fullpath, int flags);

//math.h
unsigned int pow2(unsigned int x);
unsigned int log2(unsigned int x);

//stdio.h
void scanf(char *text,void *val);
int remove(const char *filename);


//stdlib.h
int atoi (char *data);
void itoa (int val,char *char_val,unsigned int base);
void exit(int status);
void *malloc(unsigned int mem_size);
void free(void *address);
//void *bigMalloc(unsigned int mem_size);
//void bigFree(void *address);
unsigned int rand();
int getc();

//string.h
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2,unsigned int n);
unsigned int strlen(const char* s);

//unistd.h
int exec(char* path,char* argv[]);
int fork();
int pause();
int sleep(unsigned int time);
int close(int fd);
int read(int fd, void *buf, int count);
int write(int fd, void *buf, int count);
int chdir(char* path);

//stat.h
int mkdir(const char *fullpath);

//debug.h
void check_free_mem();

//sys/stat.h
int stat(char* pathname, t_stat* stat);

#endif

