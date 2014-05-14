CC = gcc

all: serverSocket clientSocket launchClient

debug: serverSocketDBG clientSocketDBG launchClient

launchClient: launchClient.c
	$(CC) -o launchClient launchClient.c

serverSocket: serverSocket.c socketMng.o
	$(CC) -o serverSocket -g serverSocket.c socketMng.o

clientSocket: clientSocket.c socketMng.o
	$(CC) -o clientSocket -g clientSocket.c socketMng.o


#### DEBUG #####
serverSocketDBG: serverSocket.c socketMngDBG.o
	$(CC) -o serverSocket -g serverSocket.c socketMng.o -DDEBUG

clientSocketDBG: clientSocket.c socketMngDBG.o
	$(CC) -o clientSocket -g clientSocket.c socketMng.o 

socketMngDBG.o: socketMng.c
	$(CC) -c -g socketMng.c -DDEBUG

clean: 
	rm -f serverSocket clientSocket socketMng.o launchClient
