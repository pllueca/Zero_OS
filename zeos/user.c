#include <libc.h>

int pid;
char buffer[32];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
	//runjp();
	pid = getpid();
	if (pid == 1){
		buffer = "proces init\n";
		write(1,buffer,srtlen(buffer));
	}
        while(1) 
    { 
 
    }
}
