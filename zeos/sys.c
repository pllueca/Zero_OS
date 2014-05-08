/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <utils.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <schedperf.h>
#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define MAX_WRITE 128
#define NULL 0

int PID_MAX = 1;

int check_fd(int fd, int permissions)
{
    if (fd!=1) return -EBADF; /*EBADF*/
    if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
    return 0;
}

int sys_ni_syscall()
{
    return -ENOSYS; /*ENOSYS*/
}


int sys_write(int fd,char * buffer, int size)
{
    int w, written;
    act_ticks_user2kernel();

    if(check_fd(fd, 1) < 0) // fd not valid
    {
        act_ticks_kernel2user();
        return -EBADF; 
    }

    if(buffer == NULL) // buffer is null
    {
        act_ticks_kernel2user();
        return -EFAULT; 
    }

    if(size < 0)  // size < 0
    {
        act_ticks_kernel2user();
        return -EINVAL; 
    }

    //tot ok, comença a escriure 

    written = 0;
    
    while(size >= MAX_WRITE)
    {
        char buff2[MAX_WRITE];
        if(copy_from_user(buffer + written, &buff2, MAX_WRITE) >= 0)
        {
            w = sys_write_console(&buff2, MAX_WRITE);
            written += w;
            size -= w;
        }
    }	  
    char buff2[size];  //size menor q max_write
    if(copy_from_user(buffer + written, &buff2, size) >= 0)
    {
        w = sys_write_console(&buff2, size);
        written += w;
        size -= w;
    }
    if(size == 0) 
    {
        act_ticks_kernel2user();
        return written;  //tot ok
    }
}


extern int zeos_ticks ;

int sys_gettime()
{
    act_ticks_user2kernel();
    act_ticks_kernel2user();
    return zeos_ticks;
}

int sys_getpid()
{
    act_ticks_user2kernel();
    act_ticks_kernel2user();
    return current()->PID;
}


/*
 * lo ejecutara un proceso recien creado al entrar en la cpu x primera vez
 */
int ret_from_fork()
{
    act_ticks_kernel2user();
    return 0;
}

/* el pcb que recibe como parametro vuelve a estar en la readyqueue */
void free_PCB(struct task_struct *t)
{
    list_add(&t->list, &freequeue);
}

/*
 * crea un proceso hijo identico al padre
 * el retorno de fork() sera 0 para el proceso hijo y el pid del nuevo proceso para el padre
 * sketch:
 * - adquirir un PCB libre de la freequeue (1)
 * - heredar la kernel stack del padre (2)
 * - copiar el espacio de memoria del padre al hijo (solo data+stack, codigo es compartido) (3)
 * - el codigo del padre de ha de mapear en la tabla de paginas del hijo!!!!!! (3.0)
 * - para esto se reservan NUM_PAG_DATA frames para el hijo (3.1), que se mapean en la tabla de paginas del padre(3.2) i del hijo(3.3)
 * - al acabar de copiar los datos se liberan las paginas del hijo de la tabla de paginas del padre (3.4)
 * - se ha de asignar a el hijo un pid q no este ocupado (max_pid + 1) (4)
 * - al entrar en ejecucion el hijo ha de pasar por "ret_from_fork" para q retorne 0, y asi identificarlo como el proceso hijo(5)
 * - actualizar la TLB (setcr3) (6)
 */
int sys_fork()
{
    int PID, child_frame, parent_frame,
            allocDir_ret, i, dir_ini, dir_dest, pos_act;
    int child_frames[NUM_PAG_DATA];
    struct list_head *l;
    struct task_struct *child, *parent;
    union task_union *child_union;
    page_table_entry *child_page, *parent_page;
    act_ticks_user2kernel();
    if(list_empty(&freequeue) != 0)
    {
        act_ticks_kernel2user();
        return -EAGAIN;
    }
    /* (1) */
    l = list_first(&freequeue);
    list_del(l);
    child = list_head_to_task_struct(l);
	
    /* (2) */
    parent = current();
    copy_data(parent,child, KERNEL_STACK_SIZE*4);

    parent_page = get_PT(parent);
    allocDir_ret = allocate_DIR(child);

    if(allocDir_ret != 1)
    {
        free_PCB(child);
        act_ticks_kernel2user();
        return -ENOMEM;
    }

    child_page = get_PT(child);

    for(i = 0; i < NUM_PAG_CODE; i++)
    {
        parent_frame = get_frame(parent_page, i + NUM_PAG_KERNEL);
        set_ss_pag(child_page, i + NUM_PAG_KERNEL, parent_frame);     /* (3.0) */
    }

    for(i = 0; i < NUM_PAG_DATA; i++)
    {
        child_frames[i] = alloc_frame();      /* (3.1)  */
        if(child_frames[i] == -1){
            while(i >= 0) 
                free_frame(child_frames[i--]);
            free_PCB(child);
            act_ticks_kernel2user();
            return -ENOMEM;
        }
    }
    for(i = NUM_PAG_KERNEL; i < NUM_PAG_DATA+NUM_PAG_KERNEL; i++)
    {              
        child_frame = child_frames[i - NUM_PAG_KERNEL];
        set_ss_pag(parent_page, i+(NUM_PAG_CODE+NUM_PAG_DATA), child_frame); /* (3.3) */
        set_ss_pag(child_page, i+(NUM_PAG_CODE), child_frame); /* (3.2) */
    }

    dir_ini = L_USER_START + (NUM_PAG_CODE)*PAGE_SIZE;
    dir_dest = L_USER_START+(NUM_PAG_CODE+NUM_PAG_DATA)*PAGE_SIZE;         
    copy_data(dir_ini, dir_dest, NUM_PAG_DATA * PAGE_SIZE);     /* (3) */   

    for(i = NUM_PAG_KERNEL; i < NUM_PAG_DATA+NUM_PAG_KERNEL; ++i)
    {
        del_ss_pag(parent_page, i+(NUM_PAG_CODE+NUM_PAG_DATA));      /* (3.4)  */
    }

    set_cr3(get_DIR(parent)); /* (6) */

    PID = PID_MAX + 1;
    child -> PID = PID;      /* (4) */
    PID_MAX = PID;

    __asm__ __volatile__
            (
                "movl %%ebp,%0;"
                :"=g"(pos_act)
             );
            
    child_union = (union task_union *) child;
    pos_act = ((unsigned int)pos_act - (unsigned int)parent) / 4; 
    child_union -> stack[pos_act] = (unsigned int) ret_from_fork; /* (5) */
    child_union -> stack[pos_act-1] = 0;
    child->kernel_esp =(unsigned int) &(child_union -> stack[pos_act-1]);
    
    /* inicialitzacions x el scheduling */
    set_ini_stats(child);
    list_add_tail(&child->list, &readyqueue);

    /* 
       Test fork 
       task_switch(child_union);
    */
    act_ticks_kernel2user();
    return PID;
}

void sys_exit()
{  
    struct task_struct *act;
    page_table_entry *act_pag;
    int i, frame_act;
    act_ticks_user2kernel();
    act = current();
    act_pag = get_PT(act);
    for(i = 0; i < NUM_PAG_DATA; i++)
    {
        frame_act = get_frame(act_pag, NUM_PAG_KERNEL+NUM_PAG_CODE + i);
        free_frame(frame_act);
    }
    act->PID = -1;  // PID = -1 -> Task morta
    update_current_state(&freequeue);
    sched_next();
}

int sys_get_stats(int pid, struct stats *st)
{
    int err;
    act_ticks_user2kernel();
    if(pid < 0) 
    {
        act_ticks_kernel2user();
        return -EINVAL;
    }

    if(st == NULL || st < L_USER_START ||
       st > L_USER_START + ((NUM_PAG_DATA + NUM_PAG_CODE)*PAGE_SIZE) ) 
    {
        act_ticks_kernel2user();
        return -EFAULT;
    }

    if(current()->PID == pid)  // si es el proces actual ens estalivem de buscar
    {
        act_ticks_kernel2user();
        copy_to_user(&current()->statics, st, sizeof(struct stats));
        return 0;
    }

    else
    {
        err = getStatPID(pid,st);
        if(err == -1)
        {
            act_ticks_kernel2user();
            return -ESRCH;
        }
        act_ticks_kernel2user();
        return 0;
    }
}

int sys_read_cutre(int fd, char *buf, int nbytes) {
  act_ticks_user2kernel();
  sys_read(fd, buf, nbytes);
  act_ticks_kernel2user();
}
