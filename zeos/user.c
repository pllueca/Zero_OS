#include <libc.h>


int pid, c_pid;
char buffer[64];


int fib(int n)
{
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fib(n-1) + fib(n-2);
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
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
            // fill 1
            for(i = 0; i < 10; ++i)
                r = read(0,&buff, 10);
            write(1,"1\n",2);
        }
        else{
            // fill 2
            for(i = 0; i < 5; ++i)
                r = read(0,&buff, 20);
            write(1,"2\n",2);
        }
    }
    else{
        // pare
        for(i = 0; i < 20; ++i)
            r = read(0,&buff, 5);
        write(1,"p\n",2);
    }
}

void workload3(){
    int pid,pid_f,f;
    pid = fork();
    if(pid == 0){
        pid_f = fork();
        if(pid_f == 0){
            // fill 1
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
    {
        workload1();
    }
    else if (n == 1)
    {
        workload2();
    }
}

/* escriu les estadistiques dels proces 1,2,3 i idle */
void print_stats()
{
    
}

int __attribute__ ((__section__(".text.main")))
main(void)
{ 
    set_sched_policy(0);
    exec_workload(2);
    //    print_stats();
    while(1);
}
