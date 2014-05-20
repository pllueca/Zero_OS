#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

/* Global vars */
int MAX_CLIENTS;
int NUM_CONECTIONS;

void func_sigchild(int signal)
{
    NUM_CONECTIONS--;
#ifdef DEBUG
    printf("acaba hijo *******\n");
    printf("Conexions: [%d/%d]\n",NUM_CONECTIONS, MAX_CLIENTS);
#endif
}

void doService (int fd) {
    int i = 0; 
    char buff[80];
    char buff2[80];
    int ret;
    int socket_fd = (int) fd;

    ret = read(socket_fd, buff, sizeof(buff));
    while(ret > 0) {
        buff[ret]='\0';
        sprintf(buff2, "Server [%d] received: %s\n", getpid(), buff);
        write(1, buff2, strlen(buff2));
        ret = write(fd, "caracola ", 8);
        if (ret < 0) {
            perror ("Error writing to socket");
            exit(1);
        }
        ret = read(socket_fd, buff, sizeof(buff));
    }
    if (ret < 0) {
        perror ("Error reading from socket");

    }
    sprintf(buff2, "Server [%d] ends service\n", getpid());
    write(1, buff2, strlen(buff2));
}


/* Unbounded socket */
void doServiceFork (int fd)
{
    int pid;
    pid = fork();
    if(pid < 0){
        perror ("Error al crear hijo\n");
        exit(1);
    }
    if(pid == 0){
        doService(fd);
        exit(0);
    }
    NUM_CONECTIONS++;
}

void *doServiceThread (void *f)
{
    int fd;
    fd = (int) f;
    doService(fd);
    pthread_exit(NULL);
}

void (*ServerLoop)(int socketFD);

void boundedServerLoop(int socketFD)
{
    int connectionFD;

#ifdef DEBUG
    printf("Conexions: [%d/%d]\n",NUM_CONECTIONS, MAX_CLIENTS);
#endif

    if(NUM_CONECTIONS >= MAX_CLIENTS)
    {

#ifdef DEBUG
        printf("Num maxim de conexions, pause\n");
#endif
        while (waitpid(-1,NULL,0) > 0);
    }
    connectionFD = acceptNewConnections (socketFD);
    if (connectionFD < 0)
    {
        perror ("Error establishing connection \n");
        deleteSocket(socketFD);
        exit (1);
    }
    doServiceFork(connectionFD);
}

void threadedServerLoop(int socketFD)
{
    int connectionFD;
    int r;
    pthread_t child_thread;
    connectionFD = acceptNewConnections(socketFD);

    r = pthread_create(&child_thread, 
                       NULL, 
                       doServiceThread, 
                       (void *) connectionFD);
    if(r){
        perror("Error creating thread\n");
        exit(1);
    }

}

void sequentialServerLoop(int socketFD)
{
    int connectionFD;
    connectionFD = acceptNewConnections (socketFD);
    if (connectionFD < 0)
    {
        perror ("Error establishing connection \n");
        deleteSocket(socketFD);
        exit (1);
    }
    doService(connectionFD);   
}

void unboundedServerLoop(int socketFD)
{
    int connectionFD;
    connectionFD = acceptNewConnections (socketFD);
    if (connectionFD < 0)
    {
        perror ("Error establishing connection \n");
        deleteSocket(socketFD);
        exit (1);
    }
    doServiceFork(connectionFD);
}

void setScheme(int s)
{
    switch (s){
        case 1:
            ServerLoop = sequentialServerLoop;
            break;
        case 2:
            ServerLoop = unboundedServerLoop;
            break;
        case 3:
            ServerLoop = boundedServerLoop;
            break;
        case 4:
            ServerLoop = threadedServerLoop;
            break;
    }
}


main (int argc, char *argv[])
{
    int socketFD;
    int connectionFD;
    char buffer[80];
    int ret;
    int port;
    int schem;
    if (argc < 3)
    {
        strcpy (buffer, "Usage: ServerSocket PortNumber ServerScheme [MaxClients]\n");
        write (2, buffer, strlen (buffer));
        exit (1);
    }
    port = atoi(argv[1]);
    schem = atoi(argv[2]);

#ifdef DEBUG
    printf("port: %s\n", argv[1]);
    printf("schem: %s\n",argv[2]);
#endif

    MAX_CLIENTS = 100;
    if(argc == 4)
        MAX_CLIENTS = atoi(argv[3]);
    NUM_CONECTIONS = 0;

    socketFD = createServerSocket (port);
    if (socketFD < 0)
    {
        perror ("Error creating socket\n");
        exit (1);
    }
    setScheme(schem);
    signal(SIGCHLD,func_sigchild);  // reprogramacio del signal sigchld, quan un fill acaba descompta el nombre de conexions totals
    while (1) {
#ifdef DEBUG
        printf("Conexions: [%d/%d]\n",NUM_CONECTIONS, MAX_CLIENTS);
#endif
        ServerLoop(socketFD);
    }
    }
