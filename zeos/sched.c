/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>



#define lh2ts list_head_to_task_struct

union task_union task[NR_TASKS]
__attribute__((__section__(".data.task")));



struct task_struct *list_head_to_task_struct(struct list_head *l)
{
    return list_entry( l, struct task_struct, list);
}
 

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
    return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
    return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
    int pos;

    pos = ((int)t-(int)task)/sizeof(union task_union);

    t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

    return 1;
}

void cpu_idle(void)
{
    int a;
    __asm__ __volatile__("sti": : :"memory");
    while(1)
    {
        printk("a");
        printk("\n");
    }
}

void init_idle (void)
{
    int e;
    struct list_head *l;
    union task_union *idle_stack;
    l = list_first(&free_queue);
    list_del(l);
    idle_task = lh2ts(l);
    idle_task-> PID = 0;
    idle_task->kernel_esp = KERNEL_ESP((union task_union *) idle_task) - 8;
    e = allocate_DIR(idle_task);
    idle_stack = (union task_union*) idle_task;
    idle_stack->stack[KERNEL_STACK_SIZE - 1] = cpu_idle; 
    idle_stack->stack[KERNEL_STACK_SIZE - 2] = 0;   // dummy
    if(e != 1){
        //restaurar &l a la free queue
    }
}

void init_task1(void)
{
    int e;
    struct page_table_entry *task1_dir;
    struct list_head *l;
    l = list_first(&free_queue);
    list_del(l);
    init_task = lh2ts(l);
    init_task -> PID = 1;
    init_task -> kernel_esp = KERNEL_ESP((union task_union *)init_task);
    e = allocate_DIR(init_task);
    if (e != 1) {
        /*pinch*/
    }
    set_user_pages(init_task);
    task1_dir = get_DIR(init_task);
    set_cr3(task1_dir);
    setTSS_tswitch((int)&((union task_union *)init_task)->stack[KERNEL_STACK_SIZE]); //esp0 de la TSS
    init_task->task_stats.user_ticks = 0;
    init_task->task_stats.system_ticks = 0;
    init_task->task_stats.elapsed_total_ticks = 0;
    init_task->task_stats.total_trans = 0;
    
    
}


void init_sched()
{
    int i;
    act_t = 0;
    INIT_LIST_HEAD(&free_queue);
    INIT_LIST_HEAD(&ready_queue);
    for(i=0; i< NR_TASKS; i++) 
        list_add(&task[i].task.list, &free_queue);
}

struct task_struct* current()
{
    int ret_value;
  
    __asm__ __volatile__(
        "movl %%esp, %0"
        : "=g" (ret_value)
                         );
    return (struct task_struct*)(ret_value&0xfffff000);
}

/* wrapper, salva els registres %esi, %edi i %ebx
   i crida a inner_task_switch
*/
void task_switch(union task_union*t)
{
    __asm__ __volatile__(
        "pushl %esi;"
        "pushl %edi;"
        "pushl %ebx;"
                         );

    inner_task_switch(t);

    __asm__ __volatile__ (
        "popl %ebx;"
        "popl %edi;"
        "popl %esi;"
		
                          );
}

void inner_task_switch(union task_union* new)
{
    int ebp,esp_new;
    struct task_struct *new_task, *act_task;
    page_table_entry *new_task_page;
  
    __asm__ __volatile__ 
            (
                " movl %%ebp, %0;"
                :"=g" (ebp)
             );
  
    act_task = current(); 
    act_task-> kernel_esp = ebp;

    new_task = (struct task_struct*) new;
    esp_new = new_task->kernel_esp;
    setTSS_tswitch((int)&(new->stack[KERNEL_STACK_SIZE])); //esp0 de la TSS
    new_task_page = get_DIR(new_task);
    set_cr3(new_task_page);
    __asm__ __volatile__
            (
                "movl %0, %%esp;"
                "popl %%ebp;"
                "ret;"
                : :"g"(esp_new)
             );
}


/* canvia la task actual x idle */
void switchIdle()
{
    task_switch(idle_task);
}

void switchInit()
{
    task_switch(init_task);
}

/* scheduling policy */

/* inicializa las estadisticas de una task */
void set_ini_stats(struct task_struct *t)
{
    t->task_stats.user_ticks = 0;
    t->task_stats.system_ticks = 0;
    t->task_stats.elapsed_total_ticks = 0;
    t->task_stats.total_trans = 0;
    t->t_state = ST_READY;
    t->task_stats.remaining_ticks = INITIAL_QUANTUM;
}


void sched_next_rr()
{
    
}


void update_current_state_rr(struct list_head *dest)
{
    
}

int needs_sched_rr()
{
    if(get_quantum(current()) <= 0)
    {
        return 1;
    }
    return 0;
}

void update_sched_data_rr()
{
    int quantum_act = get_quantum(current());
    set_quantum(current(), --quantum_act);
}

int get_quantum (struct task_struct * t)
{
    return t->quantum;
}

void set_quantum(struct task_struct * t, int new_quantum)
{
    t->quantum = new_quantum;
}
