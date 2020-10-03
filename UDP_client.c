/*
for RUN
$ gcc -pthread UDP_client.c -o client.o
$ ./client.o localhost PORT_NUM
here PORT_NUM same given in server.o
for terminate
type 'exit' + Enter-key
*/
#include<stdio.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/time.h>
#include <netdb.h>

#define TRUE 1
#define FALSE 0

void *connection_handler(void *);

    int sockfd, portno, n, valread, *new_sock,len,c;
    char buffer[1025];
    struct sockaddr_in serv_addr;

int main(int argc, char *argv[])
{

    struct hostent *server;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);

    char *hello = "Hello from client for connection"; // send this msg to server for get connection with it and for acknowledge
    sendto(sockfd, (const char *)hello, strlen(hello),MSG_CONFIRM, (const struct sockaddr *) &serv_addr,sizeof(serv_addr));
    n = recvfrom(sockfd, (char *)buffer, 1025,MSG_WAITALL, (struct sockaddr *) &serv_addr,&len); // get acknowledgement
    if (n <= 0)
    {
        perror("ERROR reading from socket: Server Disconnected");
        printf("\nBye Server\n");
        return 0;
    }
    printf("%s\n", buffer);

    pthread_t sniffer_thread;
    new_sock = malloc(1);
    *new_sock = sockfd;

    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0) // create thread for read continuously and use while in main for write
    {
        perror("Could not create thread for Reading");
        return 1;
    }

    sleep(1);
    printf("Please enter the message simultaneously\n");
    printf("-------------------------------------------------------------------\n");

    while(TRUE)
    {
	    bzero(buffer,1025);
	    fgets(buffer,1025,stdin);

        sendto(sockfd, (const char *)buffer, strlen(buffer)-1,MSG_CONFIRM, (const struct sockaddr *) &serv_addr,sizeof(serv_addr));// wrire as client enter new msg

	    sleep(1); // for better time response
    }

    close(sockfd);
    return 0;
}
void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	while(TRUE)
	{
		    bzero(buffer,1025);
            n = recvfrom(sockfd, (char *)buffer, 1025,MSG_WAITALL, (struct sockaddr *) &serv_addr,&len);//get acknowledgement from server and other client

		    if (n <= 0 || !strncmp(buffer,"exit",4)) // if sent msg is exit then terminate client, here exit msg go to server and server getback that msg to client and lead to dissconnect it
		    {
		    	perror("Client Disconnected");
		    	printf("\nBye Server\n");
		    	exit(0);
		    	break;
		    }

		    printf("--> Msg from server side: %s\n",buffer);
    	}

	free(socket_desc);
	return 0;
}
