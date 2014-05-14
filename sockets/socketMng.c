#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#define BACKLOG 10

// Create a socket and initialize it to be able to accept 
// connections.
// It returns the virtual device associated to the socket that should be used 
// in the accept system call, to get the virtual device associated to 
// the connection
//

int
createServerSocket (int port)
{
    int fd,error;
    struct sockaddr_in address;
    struct sockaddr s_addr;
    fd = socket(AF_INET, SOCK_STREAM,0);
    if (fd == -1)
    {
        perror("Error creating socket\n");
        exit(1);
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    error = bind(fd, (struct sockaddr *) &address, sizeof(s_addr));  
    if(error != 0)
    {
        perror("Error binding socket\n");
        exit(1);
    }
    error = listen(fd,BACKLOG);
    if(error == -1){
        perror("");
        exit(1);
    }

    #ifdef DEBUG
    printf("server creat [%d]\n",fd);
    #endif

    return fd;
}

/*
 Returns the file descriptor associated to the connection.
 accept system call will fill the socketAddr parameter
 with the address of the socket for the client which is requesting the
 connection, and the addrSize parameter with the size of that address.
*/
int acceptNewConnections (int socket_fd)
{
    int channel;
    struct socklen_t *len;
    struct sockaddr *dir;
    channel = accept(socket_fd, dir, len);
    if(channel == -1)
    {
        perror("error al aceptar conexion");
        exit(1);
    }

    #ifdef DEBUG
    printf("socket [%d]\n",channel);
    #endif
    return channel;
}

// Returns the socket virtual device that the client should use to access 
// the socket, if the connection is successfully established 
// and -1 in case of error
//
// Connect system call requires the address of the 
// server socket to request the connection and the size of that address.
//

int
clientConnection (char *host_name, int port)
{
    struct sockaddr_in serv_addr;
    struct hostent * hent;
    int socket_fd;
    int ret;

    #ifdef DEBUG
    printf("client conection host:[%s] port:[%d]\n",host_name, port);
    #endif

    //creates the virtual device for accessing the socket
    socket_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
        return socket_fd;
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    hent = gethostbyname(host_name);

    if (hent == NULL) {
        close (socket_fd);
	return -1;
    }

    memcpy((char *)&serv_addr.sin_addr.s_addr, 
           (char *) hent->h_addr, 
           hent->h_length);

    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = PF_INET; 

    ret = connect (socket_fd, 
                   (struct sockaddr *) &serv_addr, 
                   sizeof (serv_addr));

    if (ret < 0)
    {
        close (socket_fd);
        return (ret);
    } 

    return socket_fd;

}


int deleteSocket (int socket_fd)
{
    close (socket_fd);
}

