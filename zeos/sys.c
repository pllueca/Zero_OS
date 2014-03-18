/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define MAX_WRITE 128

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}


int sys_write(int fd,char * buffer, int size)
{
  if(check_fd(fd, 1) >= 0)
    {
    if(buffer != NULL) {
      if(size >= 0)
	{  //tot ok
	  int written = 0;
	  
	  while(size >= MAX_WRITE)
	    {
	      char buff2[MAX_WRITE];
	      if(copy_from_user(buffer + written, &buff2, MAX_WRITE) >= 0)
		{
		  int w = sys_write_console(&buff2, MAX_WRITE);
		  written += w;
		  size -= w;
		}
	    }	  
	  //if(size > 0){
	    char buff2[size];  //size menor q max_write
	    if(copy_from_user(buffer + written, &buff2, size) >= 0){
	      int w = sys_write_console(&buff2, size);
	      written += w;
	      size -= w;
	    }
	  //}
	  if(size == 0) return written;  //tot ok
	  //return -3;   //no shan escrit tots els bytes del buffer
	}
      return -22; //size< 0
    }
  return -14; //not null
  }
  return -9; // fd not valid
}


extern int zeos_ticks ;

int sys_gettime()
{
  return zeos_ticks;
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}
