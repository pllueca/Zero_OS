#include <libc.h>


int pid, c_pid;
char buffer[64];


int __attribute__ ((__section__(".text.main")))
main(void)
{ 
	struct stats st;
	int e, i;
	e = get_stats(1, st);
	for(i = 0; i < 1000; ++i){
}
	e = get_stats(1, st);
	e = st.user_ticks;
//e = 10;
	itoa(e,&buffer);
	write(1,&buffer, strlen(&buffer));
	e = st.elapsed_total_ticks;
//e = 10;
	itoa(e,&buffer);
	write(1,&buffer, strlen(&buffer));

	while(1);
}
