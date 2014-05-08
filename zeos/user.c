#include <libc.h>


int pid, c_pid;
char buffer[64];

void print_int(int a) {
  char buff[64];
  itoa(a,buff);
  write(1,buff,strlen(buff));
  write(1,"\n",1);
}


/* calcula el n-essim terme de la seq. de fibonacci */
int fib(int n)
{
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fib(n-1) + fib(n-2);
}

void foo(int n){
    int i,a;
	a = 23;
    for (i = 0 ; i < n; ++i){
	a = a + 1 - 4*i;
	}
}

/* intensiu en calcul, no es bloqueixa */
void workload1(){
    int pid,pid_f,f;
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
	    foo(20000000);
		print_act(); exit();
        }
        else{
	    foo(20000000);
	print_act(); exit();
        }
    }
    else{
	read(0,&buffer,2);
	    foo(20000000);

	print_act();
    }
}

void workload2(){
    int pid,pid_f,f,r,i;
    char buff[32];
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
            // PID 3
             r = read(0,&buff, 3000);
//	    foo(30000000);
            print_act();
		exit();
        }
        else{
            // PID 2

//	    foo(20000000);
                r = read(0,&buff, 3000);
//	    foo(200000);
	print_act(); exit();
        }
    }
    else{
	r  = read(0,&buff,1);
        r  = read(0,&buff,3000);
//	foo(30000);
	print_act();
//	exit();
    }
}

void workload3(){
    char buff[32];
    int pid,pid_f,f;
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
            // llegeix, calcula i escriu
            f = read(0, &buff,50);
            f = fib(20);
            print_int(20);
        }
        else{
            // fill 2
        }
    }
    else{
        // pare
    }
}

void exec_workload(int n)
{
    if(n == 1)
        workload1();
    else if (n == 2)
        workload2();
    else if(n == 3)
        workload3();
}

void print(struct stats *st)
{
    int n, t;
    char b[32];
    t = 0;
    n = st->user_ticks;
	write(1,"user:\n",6);
	print_int(n);
    t += n;
	write(1,"blocked:\n",9);
	n = st->blocked_ticks;
	t += n;
	print_int(n);

	write(1,"ready:\n",7);
    n = st->ready_ticks;
    t += n;
	print_int(n);

	write(1,"system:\n",8);
    n = st->system_ticks;
    t += n;
	print_int(n);

	write(1,"total:\n",7);
	print_int(t);

}

/* escriu les estadistiques dels proces 1,2,3 i idle */
void print_stats()
{
    struct stats st;
    int i,e;
    for(i = 0; i < 4 ; ++i){
        write(1,"##############\n",15);
        e = get_stats(i,&st);
        print(&st);
    }
}

void print_act()
{
	int pid,e;
	struct stats st,st2;
	pid = getpid();
	write(1,"\nPID: ",5);
	print_int(pid);
	e = get_stats(pid,&st);
	if(e != 0){
		write(1,"error\n",6);
		return;
	}
	print(&st);
	if(pid == 1){
		
		e = get_stats(0,&st2);
		write(1,"IDLE:\n",6);
		print(&st2);
	}
}

int __attribute__ ((__section__(".text.main")))
main(void)
{ 
    set_sched_policy(1);
    exec_workload(2);
    
    while(1);
}
