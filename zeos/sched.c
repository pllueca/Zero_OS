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
    init_task->quantum = INITIAL_QUANTUM;
    init_task->statics.user_ticks = 0;
    init_task->statics.system_ticks = 0;
    init_task->statics.elapsed_total_ticks = 0;
    init_task->statics.total_trans = 0;

    
    
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
  
  t->statics.user_ticks = 0;
  t->statics.system_ticks = 0;
  t->statics.elapsed_total_ticks = 0;
  t->statics.total_trans = 0;
  t->t_state = ST_READY;
  t->statics.remaining_ticks = INITIAL_QUANTUM;
  
}

void add_user_ticks(struct task_struct *t)
{
}


/*
 * realiza el task switch
 */
void sched_next_rr()
{
  struct task_struct *next;
  struct list_head *l_next;
  
  if(list_empty(&ready_queue)){
    task_switch(idle_task);
  }
  else{
    l_next = list_first(&ready_queue);
    list_del(l_next);
    next = lh2ts(l_next);
    CPU_QUANTUM = next->quantum;
    task_switch((union task_union*)next);
  }
}

/*
 * 
 */
void update_current_state_rr(struct list_head *dest)
{
  struct task_struct *act;
  act = current();
  list_add_tail(&act->list,dest);
}

/*
 * si a la task actual se le ha acabado el quantum ha de ser expulsada de la CPU
 */
int needs_sched_rr()
{
    if(CPU_QUANTUM <= 0)
    {
      return 1;
    }
    return 0;
}

void update_sched_data_rr()
{
  --CPU_QUANTUM;
}

int get_quantum (struct task_struct * t)
{
    return t->quantum;
}

void set_quantum(struct task_struct * t, int new_quantum)
{
    t->quantum = new_quantum;
}

void act_ticks_user2kernel()
{
  struct task_struct *act;
  struct stats current_s;
    int current_ticks;
	act =(struct task_struct *)current();
    current_s = act->statics;
    current_ticks = get_ticks();
    current_s.user_ticks += current_ticks - (current_s.elapsed_total_ticks);
    current_s.elapsed_total_ticks = current_ticks;
  
}

void act_ticks_kernel2user()
{
  struct task_struct *act;
  struct stats current_s;
    int current_ticks;
	act =(struct task_struct *)current();
    current_s = act->statics;
    current_ticks = get_ticks();
    current_s.system_ticks += current_ticks - (current_s.elapsed_total_ticks);
    current_s.elapsed_total_ticks = current_ticks;
  
}

void act_ticks_kernel2ready()
{
  struct task_struct *act;
  struct stats current_s;
    int current_ticks;
	act =(struct task_struct *)current();
    current_s = act->statics;
    current_ticks = get_ticks();
    current_s.system_ticks += current_ticks - (current_s.elapsed_total_ticks);
    current_s.elapsed_total_ticks = current_ticks;
	act->t_state = ST_READY;
  
}

void act_ticks_ready2kernel()
{
  struct task_struct *act;
  struct stats current_s;
    int current_ticks;
	act =(struct task_struct *)current();
    current_s = act->statics;
    current_ticks = get_ticks();
    current_s.ready_ticks += current_ticks - (current_s.elapsed_total_ticks);
    current_s.elapsed_total_ticks = current_ticks;
	act->t_state = ST_RUN;
	++current_s.total_trans;
  
}

int getStatPID(int pid, struct stats *st)
{
	struct task_struct *act;
	struct list_head *l;
    struct stats current_s;	
	int i;
	list_for_each(l, &ready_queue){
		act = lh2ts(l);
		if(act->PID == pid)
		{
			st = &(act->statics);
			return 0;
		}
	}
	return -1;
	
}

