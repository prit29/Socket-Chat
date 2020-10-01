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

#include <netdb.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0

void *connection_handler(void *);

    int sockfd, portno, n, valread, *new_sock,ret;
    char buffer[1025];
    pthread_mutex_t mymutex;

    char id[5];

int main(int argc, char *argv[])
{
    struct hostent *server;
    struct sockaddr_in serv_addr;

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
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        	perror("ERROR connecting");

	    bzero(buffer,1025);
	    n = read(sockfd,buffer,1025);

	    if (n <= 0)
	    {
	    	perror("ERROR reading from socket: Server Disconnected");
	    	printf("\nBye Server\n");
	    	return 0;
	    }
	    printf("%s\n",buffer);

	    bzero(id,5);
	    n = read(sockfd,id,5);

        pthread_t sniffer_thread;
        new_sock = malloc(1);
    	*new_sock = sockfd;

    	ret = pthread_mutex_init(&mymutex, NULL);

    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
	{
		perror("Could not create thread for Reading");
		return 1;
	}

    sleep(1);
    printf("Please enter the FTP command and file name simultaneously\n");
    printf("-------------------------------------------------------------------\n");

    while(TRUE)
    {
        printf("Enter FTP command: ");
    	char cmd[100];
    	scanf("%s",cmd);
    	n = write(sockfd,cmd,strlen(cmd));

    	if(strcmp(cmd,"GET")==0)
    	{
            printf("Enter FILE name: ");
    		bzero(buffer,1025);
			scanf("%s",buffer);

			n = write(sockfd,buffer,strlen(buffer));

			if (n <= 0)
			 perror("ERROR writing to socket");

			sleep(1); // for better time response
    	}
    	else if(strcmp(cmd,"PUT")==0)
    	{
            printf("Enter FILE name: ");
    		bzero(buffer,1025);
			scanf("%s",buffer);

			n = write(sockfd,buffer,strlen(buffer));

			if (n <= 0)
				perror("ERROR writing to socket");

			sleep(1); // for better time response

				int fd=open(buffer,O_RDONLY,S_IRUSR);
				bzero(buffer,1025);
				n = read(fd,buffer,1025);
				if ( n > 0)
				{
					printf("File opened & Size of file is: %d\n", n);
					buffer[n] = '\0';
					printf("File Data: %s",buffer);
					int valwrite = write(sockfd, buffer, strlen(buffer));
				}
				if(n==-1)
				{
					char arr[] = "No such file exists\n";
					printf("%s\n",arr);
					int valwrite = write(sockfd,arr,strlen(arr));
				}
				close(fd);

			sleep(1); // for better time response
    	}
		else if(strcmp(cmd,"LIST")==0)
		{
			printf("All files name available in server\n");
		}
		else
		{
			printf("No such FTP command exist\n\n");
		}
    }

    ret = pthread_mutex_destroy(&mymutex);
    close(sockfd);
    return 0;
}

void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	while(TRUE)
	{
		    pthread_mutex_lock(&mymutex);

		    char cmd[100];
    		bzero(cmd,100);
    		n = read(sockfd,cmd,100);

    		if (n <= 0)
		    {
		    	perror("ERROR reading from socket: Server Disconnected");
		    	printf("\nBye Server\n");
		    	exit(0);
		    	break;
		    }

		    if(strcmp(cmd,"GET")==0)
			{
				bzero(buffer,1025);
				n = read(sockfd,buffer,1025);

				if (n <= 0)
				{
					perror("ERROR reading from socket: Server Disconnected");
					printf("\nBye Server\n");
					exit(0);
					break;
				}

				printf("--> File Data: %s",buffer);

				char fname[100] = "GET_Client_";

				strcat(fname,id);
				remove(fname);
				int fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
				write(fd,buffer,strlen(buffer));
				printf("File %s saved succesfully in client\n\n",fname);
				close(fd);
			}
			else if(strcmp(cmd,"PUT")==0)
			{
				printf("File send succesfully\n\n");
			}
			else if(strcmp(cmd,"LIST")==0)
			{
					pthread_mutex_t mymux;
					int init = pthread_mutex_init(&mymux, NULL);
					pthread_mutex_lock(&mymux);

				bzero(buffer,1025);
				n = read(sockfd,buffer,1025);

				if (n <= 0)
				{
					perror("ERROR reading from socket: Server Disconnected");
					printf("\nBye Server\n");
					exit(0);
					break;
				}

				printf("--> File Data: \n%s",buffer);

				char fname[100] = "LIST_Client_";
				strcat(fname,id);
				remove(fname);
				int fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
				write(fd,buffer,strlen(buffer));
				printf("File %s saved succesfully in client\n\n",fname);
				close(fd);

					pthread_mutex_unlock(&mymux);
			}
			else
			{
				printf("No such FTP command exists, %s response from server\n",cmd);
			}

        	pthread_mutex_unlock(&mymutex);
    }

	free(socket_desc);
	return 0;
}
