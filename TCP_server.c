/*
for RUN
$ gcc -pthread TCP_server.c -o server.o
$ ./server.o PORT_NUM SERVER_TYPE &
here PORT_NUM any you want
and SERVER_TYPE 1 for OO(one-to-one) & 2 for BC(broadcast)
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

#define TRUE 1
#define FALSE 0

void *connection_handler(void *);

	int opt = TRUE;
	int allFlag=1;
	int socket_desc , new_socket , portno, c , *new_sock;
    int addrlen, client_socket[30], max_clients = 30, i, valread, sd, valwrite;
	int max_sd;
	struct sockaddr_in server , client;
    char buffer[1025];

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
	    allFlag = atoi(argv[2]);
	    if(allFlag==2)
	    	printf("Broadcast server\n");
	    else
	    	printf("One-to-one server\n");
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

        while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) //get continuously connection in while and acknowledge each client
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


			for (i = 0; i < max_clients; i++)
			{
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket; // add new client socket in list of sockets
					printf("Adding to list of sockets as %d\n" , i);
					printf("-------------------------------------------------------------------\n");
					break;
				}
			}


    		new_sock = malloc(1);
    		*new_sock = new_socket;

    		if( pthread_create( &sniffer_thread, NULL ,  connection_handler , (void*) new_sock) < 0) // create thread for each client for read continuously
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

	return 0;
}


void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;

	while(TRUE)
	{
		bzero(buffer,1025);
		valread=read(sock,buffer,1024); // read client msgs
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
	    		client_socket[flag] = 0; // if client dissconnect or close terminal

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

			printf("Msg from %d at index %d : %s\n",sock,flag,buffer);
			valwrite = write(sock , buffer , strlen(buffer)); // write acknowledge to that client

			strcat(buffer," >>>This msg from client ");
			char num[5];
			sprintf(num,"%d",sock);
			strcat(buffer,num);
			strcat(buffer,"\n");

			if(allFlag==1)
			{
				if(client_socket[flag+1]==0)
				{
					if(flag!=0)
						valwrite = write(client_socket[0] , buffer , strlen(buffer)); // write acknowledge to next other clients as condition given at starting run time
				}
				else
					valwrite = write(client_socket[flag+1] , buffer , strlen(buffer)); // write acknowledge to next other clients as condition given at starting run time
			}
			else
			{
				for (i = 0; i < max_clients; i++)
				{
					if( client_socket[i] == 0 )
					{
						break;
					}
					else
					{
						if(i!=flag)
							valwrite = write(client_socket[i] , buffer , strlen(buffer)); // write acknowledge to all other clients as condition given at starting run time
					}
				}
			}
	}

	printf("Bye Cliient %d at index %d\n\n",sock,flag);
	free(socket_desc);
	pthread_exit(0);

	return 0;
}
