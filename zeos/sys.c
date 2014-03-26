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
	    char buff2[size];  //size menor q max_write
	    if(copy_from_user(buffer + written, &buff2, size) >= 0){
	      int w = sys_write_console(&buff2, size);
	      written += w;
	      size -= w;
	    }
	    if(size == 0) return written;  //tot ok
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
  int PID, child_frame;
  list_head *l;
  task_struct *child, *parent;
  page_table_entry *child_page, *parent_page;
  parent = current();
  if(list_empty(&free_queue) == 0){  //no sta buida
    l = list_first(&free_queue);
    list_del(l);
    child = list_head_to_task_struct(l);
    /* Copia cosas */
    copy_data(parent,child, KERNEL_STACK_SIZE*4);    
    child_frame = alloc_frame();
    if(child_frame >= 0){
      /*
       * cal copia la regio de codi i de data+stack del pare al fill
       * com no es pot accedir, primer shan de mapejar les pagines del
       * fill a la page table entry del fare, usant set_ss_pag i alliberantles despres
       * amb del_ss_pag
       */
    }
    else{
      /* no frame */ 
    }    
  }
  else{
    /* no free pcb */
  }
  
  
  return PID;
}

void sys_exit()
{  
}
