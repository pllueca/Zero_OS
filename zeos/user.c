#include <libc.h>


int pid, c_pid;
char buffer[64];

void print_int(int a) {
  char buff[32];
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

void foo(){
    int i;
    (for i = 0 ; i < 1000; ++i){}
}

/* intensiu en calcul, no es bloqueixa */
void workload1(){
    int pid,pid_f,f;
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
            f = fib(500);
        }
        else{
            f = fib(750);
        }
    }
    else{
        f = fib(1000);
    }
}

void workload2(){
    int pid,pid_f,f,r,i;
    char buff[32];
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
            // fill 1
            for(i = 0; i < 10; ++i)
                r = read(0,&buff, 10);
            r = fib(5);
            print_int(5);
        }
        else{
            // fill 2
            for(i = 0; i < 5; ++i)
                r = read(0,&buff, 20);
            r = fib(5);
            print_int(5);
        }
    }
    else{
        for(i = 0; i < 20; ++i)
            r  = read(0,&buff,5);
        r = fib(5);
        print_int(5);
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
    
    n = st->user_ticks;
    t += n;
    itoa(n,b);
    write(1,b,strlen(b));
    write(1,"\n",1);

    n = st->blocked_ticks;
    t += n;
    itoa(n,b);
    write(1,b,strlen(b));
    write(1,"\n",1);

    n = st->ready_ticks;
    t += n;
    itoa(n,b);
    write(1,b,strlen(b));
    write(1,"\n",1);

    n = st->system_ticks;
    t += n;
    itoa(n,b);
    write(1,b,strlen(b));
    write(1,"\n",1);

    itoa(t,b);
    write(1,b,strlen(b));
    write(1,"\n",1);

}

/* escriu les estadistiques dels proces 1,2,3 i idle */
void print_stats()
{
    struct stats st;
    int i,e;
    for(i = 0; i < 4 ; ++i){
        write(1"##############\n",15);
        e = get_stats(i,&st);
        print(&st);
    }
}

int __attribute__ ((__section__(".text.main")))
main(void)
{ 
    set_sched_policy(0);
    exec_workload(2);
    print_stats();
    while(1);
}
