#include <libc.h>

int pid;
char buffer[32];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
    //runjp();
    pid = 203;
    pid = getpid();
    itoa(pid,&buffer);
    write(1,"pid\n",4);
    write(1,buffer,strlen(buffer));
    while(1) 
    { 
      
    }
}
