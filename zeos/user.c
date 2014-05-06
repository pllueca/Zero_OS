#include <libc.h>


int pid, c_pid;
char buffer[64];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
    set_sched_policy(0);
    while(1);
}
