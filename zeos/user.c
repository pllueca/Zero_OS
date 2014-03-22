#include <libc.h>

int pid;
char buffer[32];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
	//runjp();
	pid = getpid();
	if (pid == 1){
		write(1,"pid",3);
	}
	while(1) 
    { 
 
    }
}
