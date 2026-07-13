#ifndef WALTER_WINDOW_MANAGER
#define WALTER_WINDOW_MANAGER

//some useful things to define
//file descriptors
#define WWM_STDIN_FILENO 	0
#define WWM_STDOUT_FILENO 	1
#define WWM_STDERR_FILENO 	2
//open() flags
#define WWM_O_RDONLY	0x0000
#define WWM_O_WRONLY	0x0001
#define WWM_O_RDWR	0x0002
#define WWM_O_CREAT	0x0040
#define WWM_O_EXCL	0x0080
#define WWM_O_TRUNC	0x0200
#define WWM_O_APPEND 	0x0400
#define WWM_O_NONBLOCK	0x0800
#define WWM_O_DIRECTORY	0x10000
#define WWM_O_CLOEXEC	0x80000
//mmap() protection flags
#define WWM_PROT_NONE	0x0 //0000
#define WWM_PROT_READ	0x1 //0001
#define WWM_PROT_WRITE	0x2 //0010
#define WWM_PROT_EXEC	0x4 //0100
//mmap() flags
#define WWM_MAP_SHARED 	0x01
#define WWM_MAP_PRIVATE	0x02
#define WWM_MAP_FIXED	0x10
#define WWM_MAP_ANONYMOUS	0x20
#define WWM_MAP_ANON 	MAP_ANONYMOUS
//seek()
#define WWM_SEEK_SET	0
#define WWM_SEEK_CUR	1
#define WWM_SEEK_END	2
//syscall numbers
#define WWM_SYS_read 	0
#define WWM_SYS_write	1
#define WWM_SYS_open	2

//uints type to use for various things
typedef unsigned char uints;
typedef unsigned int uint;
typedef unsigned long uintl;

//uses wwm-lib.s asm source file _syscall to make
//system calls without cstdlib
//that way wwmlib can be a static library/archive
//without taking up a whole ass megabyte son
extern long _syscall(long syscallNumber,
		long arg0,
		long arg1,
		long arg2,
		long arg3,
		long arg4,
		long arg5);
//a couple useful system calls for you :)
//prints to stdout, NOT FORMATTED!!!!
long print(char* string);
//write system call
long write(uints fd, char* string, uintl size);
//open system call
long open(char* filename, int flags, long mode);
//close system call
long close(uintl fd);
//mmap system call
long mmap(void *addr, uint len, int prot, int flags, int filedes, uint off);
//exit system call
long exit(int status);
//socket system call
long socket(int domain, int type, int protocol);
//getpid system call
long getpid(void);

typedef struct WWMWindow WWMWindow;

typedef struct Cells Cells;



#endif
