/*
 * libc.c 
 */

#include <libc.h>
#include <errno.h>
#include <types.h>

int errno;


int abs(int a)
{
    if(a < 0) return -a;
    return a;
}


//wrapper for the write system call
int write(int fd, char* buffer, int size)
{
    int e;
    __asm__ __volatile__ (
        "pushl %%ebx ;"
        "movl 8(%%ebp),%%ebx ;"
        "movl 12(%%ebp),%%ecx ;"
        "movl 16(%%ebp),%%edx;"
        "movl $4, %%eax;"
        "int $0x80;"
        "popl %%ebx;" 
        "movl %%eax, %0"
        :"=g" (e)
                          );

    if(e < 0){
        errno = abs(e);
        return -1;
    }
    return e;
}

int gettime()
{
    __asm__ __volatile__ (
        "movl $10, %eax;"
        "int $0x80;" 
                          );
    register int e asm("eax");
    if(e < 0)
    {
        errno = abs(e);
        return -1;
    }
    return 0;
}

void itoa(int a, char *b)
{
    int i, i1;
    char c;
  
    if (a==0) { b[0]='0'; b[1]=0; return ;}
  
    i=0;
    while (a>0)
    {
        b[i]=(a%10)+'0';
        a=a/10;
        i++;
    }
  
    for (i1=0; i1<i/2; i1++)
    {
        c=b[i1];
        b[i1]=b[i-i1-1];
        b[i-i1-1]=c;
    }
    b[i]=0;
}

// escriu errno
void perror()
{
    switch(errno)
    {
        case EINVAL:
            write(1,"size invalid\n",13);
            break;
        case EBADF:
            write(1,"error fd\n",6);
            break;
        case EACCES:
            write(1,"permision denied\n",17);
            break;
        case EFAULT:
            write(1,"buffer apunta a NULL\n",17);
            break;
        case 4:
            write(1,"error al gettime()\n",19);
            break;
        case ENOSYS:
            write(1,"not implemented\n",16);
            break;
    }
}

int strlen(char *a)
{
    int i;
  
    i=0;
  
    while (a[i]!=0) i++;
  
    return i;
}

int getpid()
{
    int pid;
    __asm__ __volatile__ (
        "movl $20, %%eax;"
        "int $0x80;"
        "movl %%eax, %0;"
        :"=g"(pid)
                          );
    return pid;	
}

int ret_from_fork()
{
    return 0;
}

int fork()
{
    __asm__ __volatile__ (
        "movl $2, %eax;"
        "int $0x80;"
                          );
}
