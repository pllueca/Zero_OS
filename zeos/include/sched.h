/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <libc.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define INITIAL_QUANTUM

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
    int PID;			/* Process ID */
    page_table_entry * dir_pages_baseAddr;
    int quantum;
    int kernel_esp;
    struct list_head list;
    struct stats task_stats;
    enum state_t t_state;
};

union task_union {
    struct task_struct task;
    unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
struct task_struct *idle_task;
struct task_struct *init_task;

struct list_head free_queue;
struct list_head ready_queue;


int act_t; /* 0-> init, 1-> idle*/



#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void inner_task_switch(union task_union*t);

void task_switch(union task_union*t);

void switchIdle();

void switchInit();

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void set_ini_stats(struct task_struct *t);

void sched_next_rr();
void update_current_state_rr(struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

/* quantum */
int get_quantum (struct task_struct * t);
void set_quantum(struct task_struct * t, int new_quantum);

#endif  /* __SCHED_H__ */
