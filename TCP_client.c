/*
for RUN
$ gcc -pthread 201801261_client_stream.c -o client.o
$ ./client.o localhost PORT_NUM
here PORT_NUM same given in server.o
for terminate
close tab or window where client run - message shown in server
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

    int sockfd, portno, n, valread, *new_sock,ret;
    char buffer[1025];
    pthread_mutex_t mymutex;

int main(int argc, char *argv[])
{

    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        perror("ERROR opening socket");

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        	perror("ERROR connecting");

	    bzero(buffer,1025);
	    n = read(sockfd,buffer,1025); // read 1st time when connect to server

	    if (n <= 0)
	    {
	    	perror("ERROR reading from socket: Server Disconnected");
	    	printf("\nBye Server\n");
            close(sockfd);
	    	return 0;
	    }
	    printf("%s\n",buffer);

        pthread_t sniffer_thread; // create thread for read continuously and use while in main for write
        new_sock = malloc(1);
    	*new_sock = sockfd;

    ret = pthread_mutex_init(&mymutex, NULL); // mutex for avoid RC in thread

    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
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

	    n = write(sockfd,buffer,strlen(buffer)-1); // wrire as client enter new msg

	    if (n <= 0)
		 perror("ERROR writing to socket");

	    sleep(1); // for better time response
    }

    ret = pthread_mutex_destroy(&mymutex); // mutex assign
    close(sockfd);
    return 0;
}

void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	while(TRUE)
	{
		    pthread_mutex_lock(&mymutex); // for avoid RC - lock
		    bzero(buffer,1025);
		    n = read(sockfd,buffer,1025); //get acknowledgement from server and other client

		    if (n <= 0)
		    {
		    	perror("ERROR reading from socket: Server Disconnected");
		    	printf("\nBye Server\n");
                close(sock);
		    	exit(0);
		    	break;
		    }

		    printf("--> Msg from server side: %s\n",buffer);
        	pthread_mutex_unlock(&mymutex); // unlock
    }

	free(socket_desc);
	return 0;
}
