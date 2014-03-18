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
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
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
    idle_task->PID = 0;
    e = allocate_DIR(idle_task);
    idle_stack = (union task_union*) idle_task;
  
    // @cpu_idle apunta a la direccio de codi de cpu idle
    idle_stack->stack[KERNEL_STACK_SIZE - 1] = &cpu_idle; //pos 1023
    idle_stack->stack[KERNEL_STACK_SIZE - 2] = 0;          //ebp register when undoing dynamic link
    idle_task->kernel_esp = 0;
    if(e != 1){
	//restaurar &l a la free queue
    }


}

void init_task1(void){
	int e;
	struct task_struct *task1;
	struct page_table_entry *task1_dir;
	struct list_head *l;
	l = list_first(&free_queue);
	list_del(l);
	task1 = lh2ts(l);
	e = allocate_DIR(task1);
	if (e != 1) {
	/*pinch*/
	}
	set_user_pages(task1);
	task1_dir = get_DIR(task1);
	set_cr3(task1_dir);
}


void init_sched(){
	int i;
	INIT_LIST_HEAD(&free_queue);
	INIT_LIST_HEAD(&ready_queue);
	for(i=0; i< NR_TASKS; i++) {
		list_add(&task[i].task.list, &free_queue);
	}

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


void task_switch(union task_union*t) {
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

void inner_task_switch(union task_union* new) {
	int esp,ss;
	struct task_struct *new_task;
	page_table_entry *new_task_page;
	new_task = (struct task_struct*) new;
	esp = new_task -> kernel_esp;
	ss = &new;
	setTSS_tswitch(esp,ss);
	new_task_page = get_DIR(new_task);
	set_cr3(new_task_page);

	//ebp al current PCB	
}
