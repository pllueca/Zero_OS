#include <io.h>
#include <utils.h>
#include <list.h>
#include <libc.h>
#include <sched.h>

// Queue for blocked processes in I/O 
struct list_head blocked;
extern char char_map[];
extern int act_t;


int sys_write_console(char *buffer,int size)
{
    int i;
  
    for (i=0; i<size; i++)
        printc(buffer[i]);
  
    return size;

}

extern void switchInit();
extern void switchIdle();

int keyboard_routine()
{
    char key = inb(0x60);
    char m = key & (0x7F);
    if (!(key & 0x80)) 
    {
        char t;      
        t = char_map[m];
        if(t == '\0') 
            t = 'C';
        printc_xy(0,0,t);
    }
    return 1;
}

extern int zeos_ticks;

int clock_routine()
{
    ++zeos_ticks;
    update_sched_data_rr();
    if(needs_sched_rr)
    {
        
    }
    zeos_show_clock();
    return 0;
}
