/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <schedperf.h>
#include <mm.h>
#include <io.h>
#include <errno.h>


#define lh2ts list_head_to_task_struct
#define RR 0
#define FCFS 1

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

    }
}

void init_idle (void)
{
    int e;
    struct list_head *l;
    union task_union *idle_stack;
    l = list_first(&freequeue);
    list_del(l);
    idle_task = lh2ts(l);
    idle_task-> PID = 0;
    idle_task->kernel_esp = KERNEL_ESP((union task_union *) idle_task) - 8;
    e = allocate_DIR(idle_task);
    if(e != 1)
        return -EDNALL;
    else
    {
        idle_stack = (union task_union*) idle_task;
        idle_stack->stack[KERNEL_STACK_SIZE - 1] = cpu_idle; 
        idle_stack->stack[KERNEL_STACK_SIZE - 2] = 0;   // dummy
	
    }
}

void init_task1(void)
{
    int e;
    struct page_table_entry *task1_dir;
    struct list_head *l;
    l = list_first(&freequeue);
    list_del(l);
    init_task = lh2ts(l);
    init_task -> PID = 1;
    init_task -> kernel_esp = KERNEL_ESP((union task_union *)init_task);
    e = allocate_DIR(init_task);
    if (e != 1)  
        return -EDNALL;
    set_user_pages(init_task);
    task1_dir = get_DIR(init_task);
    setTSS_tswitch((int)&((union task_union *)init_task)->stack[KERNEL_STACK_SIZE]); //esp0 de la TSS
    set_cr3(task1_dir);

    init_task->quantum = INITIAL_QUANTUM;
    init_task->statics.user_ticks = 0;
    init_task->statics.system_ticks = 0;
    init_task->statics.blocked_ticks = 0;
    init_task->statics.elapsed_total_ticks = get_ticks();
    init_task->statics.total_trans = 0;
    init_task->t_state = ST_RUN;    
}


void init_sched()
{
    int i;
    act_t = 0;
    INIT_LIST_HEAD(&freequeue);
    INIT_LIST_HEAD(&readyqueue);
    for(i=0; i< NR_TASKS; i++) {
        list_add(&task[i].task.list, &freequeue);
    }
    init_sched_policy();
    set_sched_policy(RR);
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

/* 
   wrapper, salva els registres %esi, %edi i %ebx
   i crida a inner_task_switch
*/
void task_switch(union task_union*t)
{
    act_ticks_kernel2ready();
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
    act_ticks_ready2kernel();
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
    act_task -> kernel_esp = ebp;
    new_task = (struct task_struct*) new;
    esp_new = new_task -> kernel_esp;
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
    t->statics.blocked_ticks = 0;
    t->statics.elapsed_total_ticks = get_ticks();  // els elapsed quan comença un proces
    t->statics.total_trans = 0;
    t->t_state = ST_READY;
    t->statics.remaining_ticks = INITIAL_QUANTUM;
  
}

/*
 * realiza el task switch
 */
void sched_next_rr()
{
    struct task_struct *next;
    struct list_head *l_next;
  
    if(list_empty(&readyqueue))
    {
        task_switch(idle_task);
    }
    else
    {
        l_next = list_first(&readyqueue);
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

/* actualitza les estadistiques pasant de mode usuari a sistema  */
void act_ticks_user2kernel()
{
    struct task_struct *act;
    struct stats *current_s;
    int current_ticks;
    act =(struct task_struct *)current();
    current_s = &act->statics;
    current_ticks = get_ticks();
    current_s->user_ticks += current_ticks - (current_s->elapsed_total_ticks);
    current_s->elapsed_total_ticks = current_ticks;
}

/* actualitza les estadistiques pasant de mode sistema a usuari */
void act_ticks_kernel2user()
{
    struct task_struct *act;
    struct stats *current_s;
    int current_ticks;
    act =(struct task_struct *)current();
    current_s = &act->statics;
    current_ticks = get_ticks();
    current_s->system_ticks += current_ticks - (current_s->elapsed_total_ticks);
    current_s->elapsed_total_ticks = current_ticks;
}

/* 
 * actualitza les estadistiques pasant de mode sistema a la cua de ready,
 * quan el proces es expulsat de la CPU
 */
void act_ticks_kernel2ready()
{
    struct task_struct *act;
    struct stats *current_s;
    int current_ticks;
    act =(struct task_struct *)current();
    current_s = &act->statics;
    current_ticks = get_ticks();
    current_s->system_ticks += current_ticks - (current_s->elapsed_total_ticks);
    current_s->elapsed_total_ticks = current_ticks;
    act->t_state = ST_READY;
}

/* actualitza les estadistiques pasant de la cua de ready a executarse a la CPU */
void act_ticks_ready2kernel()
{
    struct task_struct *act;
    struct stats *current_s;
    int current_ticks;
    act =(struct task_struct *)current();
    current_s = &act->statics;
    current_ticks = get_ticks();
    current_s->ready_ticks += current_ticks - (current_s->elapsed_total_ticks);
    current_s->elapsed_total_ticks = current_ticks;
    act->t_state = ST_RUN;
    ++current_s->total_trans;
}

int getStatPID(int pid, struct stats *st)
{
    struct task_struct *act;
    struct list_head *l;
    struct stats *current_s;	
    int i;
    for(i = 0; i < NR_TASKS; ++i) 
    {
        act =(struct task_struct*) &task[i];
        if(act->PID == pid)
        {
            copy_to_user(&act->statics, st, sizeof(struct stats));
            return 0;
        }
    }
    return -1;
}


struct stats * get_task_stats(struct task_struct *t)
{
    return &t->statics;
}

struct list_head * get_task_list(struct task_struct *t)
{
    return &t->list;
}


void block_process(struct list_head *block_queue)
{
    struct task_struct *act;
    struct stats *st;
    act = current();
    st = get_task_stats(act);
    update_current_state(block_queue);
    st->system_ticks = get_ticks() - st->elapsed_total_ticks;
    st->elapsed_total_ticks = get_ticks();
    //    act->t_state = ST_BLOCKED; ?
    sched_next();
}

void unblock_process(struct task_struct *blocked)
{
    struct stats *st;
    struct list_head *l;
    st = get_task_stats(blocked);
    l = get_task_list(blocked);
    list_del(l);
    list_add_tail(l, &readyqueue);
    st->blocked_ticks += (get_ticks()-st->elapsed_total_ticks);
    st->elapsed_total_ticks = get_ticks();
    if (needs_sched()) {
        update_current_state(&readyqueue);
        sched_next();
    }  
}
