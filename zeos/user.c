#include <libc.h>


int pid, c_pid;
char buffer[32];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
	runjp_rank(18,80);
        while(1);
}
