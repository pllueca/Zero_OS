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
