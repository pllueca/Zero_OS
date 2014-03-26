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

int PID_MAX = 1;

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


/*
 * lo ejecutara un proceso recien creado al entrar en la cpu x primera vez
 */
int ret_from_fork()
{
  return 0;
}

int sys_fork()
{
  int PID, child_frame, allocDir_ret;
  list_head *l;
  task_struct *child, *parent;
  page_table_entry *child_page, *parent_page;
  parent = current();

  // get a free PCB for the child
  if(list_empty(&free_queue) == 0){  //no sta buida
    l = list_first(&free_queue);
    list_del(l);
    child = list_head_to_task_struct(l);

    // inheritance of parent kernel stack
    copy_data(parent,child, KERNEL_STACK_SIZE*4);    
    allocDir_ret = allocate_DIR(child);
    if(allocDir_ret != 1){    
      child_frame = alloc_frame();
      if(child_frame >= 0){
	/*
	 * cal copiar la regio de codi i de data+stack del pare al fill
	 * com no es pot accedir, primer shan de mapejar les pagines del
	 * fill a la page table entry del fare, usant set_ss_pag i alliberantles despres
	 * amb del_ss_pag
	 */
	child_page = get_PT(child);
	set_ss_pag(parent_page, "?" ,child_frame); //
	copy_data();
	del_ss_pag(parent_page, "?");
	
	PID = PID_MAX + 1;  // no hay PIDs repetidos
	PID_MAX = PID;

	child->kernel_esp = &ret_from_fork; // el hijo empezara a ejecutarse saliendo de fork con valor de retorn 0
	list_add_tail(child.list, &freequeue);  
	return PID;
      }
      else{
	/* no frame */ 
      }    
    }
    else{
      /* no directory allocated */
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
