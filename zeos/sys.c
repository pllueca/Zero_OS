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


/*
 * crea un proceso hijo identico al padre
 * el retorno de fork() sera 0 para el proceso hijo y el pid del nuevo proceso para el padre
 * sketch:
 * - adquirir un PCB libre de la freequeue (1)
 * - heredar la kernel stack del padre (2)
 * - copiar el espacio de memoria del padre al hijo (solo data+stack, codigo es compartido) (3)
 * - para esto se reservan NUM_PAG_DATA frames para el hijo (3.1), que se mapean en la tabla de paginas del padre(3.2) i del hijo(3.3)
 * - al acabar de copiar los datos se liberan las paginas del hijo de la tabla de paginas del padre (3.4)
 * - se ha de asignar a el hijo un pid q no este ocupado (max_pid + 1) (4)
 * - al entrar en ejecucion el hijo ha de pasar por "ret_from_fork" para q retorne 0, y asi identificarlo como el proceso hijo (5)
 * - en algun momento de hace un flush de la tlb (set_cr3)
 */
int sys_fork()
{
    int PID, child_frame,
            allocDir_ret, i, dir_ini, dir_dest, pos_act;
    struct list_head *l;
    struct task_struct *child, *parent;
    union task_union *child_union;
    page_table_entry *child_page, *parent_page;
    parent = current();
    if(list_empty(&free_queue) == 0)
    {
        //parent_dir = get_DIR(parent);
        /* (1) */
        l = list_first(&free_queue);
        list_del(l);
        child = list_head_to_task_struct(l);
	
        /* (2) */
        copy_data(parent,child, KERNEL_STACK_SIZE*4);

        parent_page = get_PT(parent);
        allocDir_ret = allocate_DIR(child);
       
        child_page = get_PT(child);
  
        if(allocDir_ret == 1)
        {
            /* (3.1)  */
            for(i = NUM_PAG_KERNEL; i < NUM_PAG_DATA+NUM_PAG_KERNEL; ++i)
            {               
                child_frame = alloc_frame(); 
                if(child_frame != -1){

                
                    set_ss_pag(parent_page, i+(NUM_PAG_CODE+NUM_PAG_DATA), child_frame); /* (3.3) */
                
                    set_ss_pag(child_page, i+(NUM_PAG_CODE), child_frame); /* (3.2) */
             

                }
                else
                {
		  
                }
            }
            dir_ini = L_USER_START + (NUM_PAG_CODE)*PAGE_SIZE;
            dir_dest = L_USER_START+(NUM_PAG_CODE+NUM_PAG_DATA)*PAGE_SIZE;
            //dir_dest = dir_ini + (NUM_PAG_DATA * PAGE_SIZE);
            /* (3) */
              
            

            
            copy_data(dir_ini, dir_dest, NUM_PAG_DATA * PAGE_SIZE);
            sys_write(1,"aaa\n", 4); 

            /* (3.4)  */
            for(i = NUM_PAG_KERNEL; i < NUM_PAG_DATA+NUM_PAG_KERNEL; ++i)
            {
                del_ss_pag(parent_page, i+(NUM_PAG_CODE+NUM_PAG_DATA)); 
            }
            set_cr3(get_DIR(parent));
            /* (4) */
            PID = PID_MAX + 1;
            child -> PID = PID;
            PID_MAX = PID;
      
            /* (5) */


        
            __asm__ __volatile__(
                "movl %%ebp,%0;"
                :"=g"(pos_act));
            
            child_union = (union task_union *) child;
            pos_act = ( pos_act - (int)&(child_union))/4; 
            child_union -> stack[pos_act] = ret_from_fork;
            child_union -> stack[pos_act - 1] = 0;
            child->kernel_esp =(int) &(child_union -> stack[pos_act - 1]);
            list_add_tail(&(child->list), &ready_queue);
        }
        else
        {
            /* no directory allocated */
        }
    }
    else
    {
        /* no free pcb */
    }
    task_switch(child_union);
    return PID;
    
}

void sys_exit()
{  
}
