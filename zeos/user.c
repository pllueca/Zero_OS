#include <libc.h>

int pid, c_pid;
char buffer[32];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
    pid = getpid();
    c_pid = fork();
    if(c_pid == 0)
    {
        write(1,"hijo!\n",6);
    }
    else if(c_pid > 0)
    {
        write(1,"er papa\n",8);   
    }
    else
    {
        write(1,"sha cagao\n",10);
    }
    while(1) 
    { 
    }
}
