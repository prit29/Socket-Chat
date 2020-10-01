/*
for RUN
$ gcc -pthread 201801261_server_stream.c -o server.o
$ ./server.o PORT_NUM
here PORT_NUM any you want
for terminate
close tab or window where server run - message shown in all client and all client terminate as well
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
#include <dirent.h>
#include <netdb.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0

void *connection_handler(void *);

	int opt = TRUE;
	int allFlag=1;
	int socket_desc , new_socket , portno, c , *new_sock;
    int addrlen, client_socket[30], max_clients = 30, i, valread, sd, valwrite, ret;
	int max_sd;
	struct sockaddr_in server , client;
   	char buffer[1025];
    int BUFFSIZE = 1024;
    pthread_mutex_t mymutex;

int main(int argc , char *argv[])
{
    for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);

	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

    if( setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	    bzero((char *) &server, sizeof(server));
	    portno = atoi(argv[1]);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( portno );

	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("Bind failed");
		return 1;
	}

    if (listen(socket_desc, 5) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

    	pthread_t sniffer_thread;

        while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    	{
    		printf("-------------------------------------------------------------------\n");
            	printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(client.sin_addr) , ntohs
				(client.sin_port));

    		char message[1025] = "Hello Client , I have received your connection. Your ID : ";
    			char num[5];
			sprintf(num,"%d",new_socket);
			strcat(message,num);
			strcat(message,"\n");
    		write(new_socket , message , strlen(message));

            	puts("Welcome message sent successfully");
            	usleep(1);

            write(new_socket , num, strlen(num));


			for (i = 0; i < max_clients; i++)
			{
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
					printf("-------------------------------------------------------------------\n");
					break;
				}
			}


    		new_sock = malloc(1);
    		*new_sock = new_socket;

			ret = pthread_mutex_init(&mymutex, NULL);
    		if( pthread_create( &sniffer_thread, NULL ,  connection_handler , (void*) new_sock) < 0)
    		{
    			perror("Could not create thread");
    			return 1;
    		}

    	}
    if (new_socket<0)
	{
		perror("Accept failed");
		return 1;
	}
	ret = pthread_mutex_destroy(&mymutex);
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
		valread=read(sock,cmd,99);

		int flag = 0;
			for (i = 0; i < max_clients; i++)
			{
				if( client_socket[i] == sock )
				{
					flag = i;
					break;
				}
			}

			if (valread <= 0)
	    	{
	    		client_socket[flag] = 0;

	    		for (i = flag; i < max_clients; i++)
				{
					if( client_socket[i+1] == 0 )
					{
						client_socket[i] = 0;
						break;
					}
					else
					{
						client_socket[i] = client_socket[i+1];
					}
				}

	    		printf("Bye Cliient %d at index %d\n\n",sock,flag);
				free(socket_desc);
	    		pthread_exit(0);
	    	}

	    	if(strcmp(cmd,"GET")==0)
	    	{

	    		bzero(buffer,1025);
				valread=read(sock,buffer,1024);

				printf("Msg from %d at index %d and File name is: %s & FTP is: %s\n",sock,flag,buffer,cmd);

				valwrite = write(sock,cmd,strlen(cmd));
				sleep(1);

				int fd;
				fd=open(buffer,O_RDONLY,S_IRUSR);

				int n;
				bzero(buffer,1025);
				n = read(fd,buffer,BUFFSIZE-1);
				if ( n > 0)
				{
					printf("File opened & Size of file is: %d\n", n);
					buffer[n] = '\0';
					printf("File Data: %s\n",buffer);
					valwrite = write(sock , buffer , strlen(buffer));
				}
				if(n==-1)
				{
					char arr[] = "No such file exists\n";
					printf("%s\n",arr);
					valwrite = write(sock,arr,strlen(arr));
				}
	    	}
	    	else if(strcmp(cmd,"PUT")==0)
	    	{

	    		bzero(buffer,1025);
				valread=read(sock,buffer,1024);

				printf("Msg from %d at index %d and File name is: %s & FTP is: %s\n",sock,flag,buffer,cmd);

				bzero(buffer,1025);
				valread = read(sock, buffer,1025);

				valwrite = write(sock,cmd,strlen(cmd));
				sleep(1);

				if (valread <= 0)
				{
					perror("ERROR reading from socket: Server Disconnected");
					printf("\nBye Server\n");
					exit(0);
					break;
				}

				printf("--> File Data: %s",buffer);

				char fname[100] = "PUT_Server_";
				char ss[20];
				sprintf(ss,"%d",sock);
				strcat(fname,ss);
				remove(fname);
				int fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
				write(fd,buffer,strlen(buffer));
				printf("File %s saved succesfully in server\n\n",fname);
				close(fd);
	    	}
	    	else if(strcmp(cmd,"LIST")==0)
	    	{
	    		valwrite = write(sock,cmd,strlen(cmd));
				sleep(1);

	    		printf("Msg from %d at index %d and FTP is: %s\n",sock,flag,cmd);
	    		struct dirent *de;
				DIR *dr = opendir(".");
				if (dr == NULL)
				{
					char result[50] = "Could not open current directory";
					printf("%s\n", result);
					write(sock, result, sizeof(result));
				}
				else
				{
					pthread_mutex_t mymux;
					int init = pthread_mutex_init(&mymux, NULL);
					pthread_mutex_lock(&mymux);
					char filesname[1024];
					while ((de = readdir(dr)) != NULL)
					{
						char string[50];
						strcpy(string, de->d_name);
						if(strcmp(string,".")==0)
							continue;
						if(strcmp(string,"..")==0)
							continue;
						strcat(filesname,string);
						strcat(filesname,"\n");
					}
					write(sock, filesname, sizeof(filesname));
					closedir(dr);
					printf("List of all File name send succesfully\n\n");
					pthread_mutex_unlock(&mymux);
				}

			}
			else
			{
				printf("No such FTP command exists: %s\n\n",cmd);
			}
			pthread_mutex_unlock(&mymutex);
	}

	printf("\nBye Cliient : %d\n",sock);
	free(socket_desc);
	pthread_exit(0);

	return 0;
}
