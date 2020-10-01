/*
for RUN
$ gcc 201801261_server_datagram.c -o server.o
$ ./server.o PORT_NUM SERVER_TYPE &
here PORT_NUM any you want
and SERVER_TYPE 1 for OO(one-to-one) & 2 for BC(broadcast)
for terminate
close tab or window where server run - it doesn't give any message to client because this server is datagram that doesn't reap
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
	int socket_desc , new_socket , portno, c , *new_sock, sockfd;
    int addrlen, max_clients = 30, i, valread, sd, valwrite, n;
	int max_sd;
	struct sockaddr_in server , client, client_addr[30];
    char buffer[1025];

int main(int argc , char *argv[])
{
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	bzero((char *) &server, sizeof(server));
	bzero((char *) &client, sizeof(client));
    portno = atoi(argv[1]);
    allFlag = atoi(argv[2]);
    if(allFlag==2)
        printf("Broadcast server\n");
    else
        printf("One-to-one server\n");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( portno );

	if(bind(sockfd, (const struct sockaddr *)&server,sizeof(server)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	puts("Waiting for incoming connections...");
	c = sizeof(client);

	while(TRUE)
    {
    	bzero(buffer,1025);
    	n = recvfrom(sockfd, (char *)buffer, 1025, MSG_WAITALL, ( struct sockaddr *) &client, &c); // continuously read and write acknowledge in main while loop

    	if(n<=0 || !strncmp(buffer,"exit",4)) //for termination info but in UDP it does not give any response so client termination info wan't be reaped, for terminate type exit in client socket
    	{
    			int ff=0;
    			for (i = 0; i < max_clients; i++)
                {
                    if( (client_addr[i].sin_addr.s_addr==client.sin_addr.s_addr) && (client_addr[i].sin_port==client.sin_port))
                    {
                        ff = i;
                        char buzz[1025] = "exit";
                        sendto(sockfd, (const char *)buzz, strlen(buzz),MSG_CONFIRM, (const struct sockaddr *) &client,c);
                        break;
                    }
                }

	    		for (i = ff; i < max_clients; i++)
    			{
    				if( (client_addr[i].sin_addr.s_addr=='\0') && (client_addr[i].sin_port=='\0') )
    				{
    					break;
    				}
    				else
    				{
    					client_addr[i] = client_addr[i+1];
    				}
    			}

	    		printf("Bye Cliient at index %d\n\n",ff);

    	}
    	else
    	{
    			int ff=-1;
    			for (i = 0; i < max_clients; i++)
                {
                    if( (client_addr[i].sin_addr.s_addr==client.sin_addr.s_addr) && (client_addr[i].sin_port==client.sin_port))
                    {
                        ff = i;
                        break;
                    }
                }

                if(ff==-1) // if client sockaddr_in not found then acknowledge client and form up new connection add sockaddr_in in list of it
                {
                	printf("-------------------------------------------------------------------\n");
		            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , sockfd , inet_ntoa(client.sin_addr) , ntohs(client.sin_port));

		            for (i = 0; i < max_clients; i++)
		            {
		                if( (client_addr[i].sin_addr.s_addr=='\0') && (client_addr[i].sin_port=='\0'))
		                {
		                    char message[1025] = "Hello Client , I have received your connection at location: ";
						    char num[5];
						    sprintf(num,"%d",i);
						    strcat(message,num);
						    strcat(message,"\n");
						    sendto(sockfd, (const char *)message, strlen(message),MSG_CONFIRM, (const struct sockaddr *) &client,c);

						    puts("Welcome message sent successfully");

						    client_addr[i] = client;
		                    printf("Adding to list of sockets at %d\n" , i);
		                    printf("-------------------------------------------------------------------\n");

		                    break;
		                }
		            }
                }
                else // if client found in list then write acknowledge and print here as well
                {
                	printf("Msg from client at index %d : %s\n",ff,buffer);
            		sendto(sockfd, (const char *)buffer, strlen(buffer),MSG_CONFIRM, (const struct sockaddr *) &client,c);// write acknowledge to that client

            		strcat(buffer," >>>This msg from client ");
					char num[5];
					sprintf(num,"%d",ff);
					strcat(buffer,num);
					strcat(buffer,"\n");

					if(allFlag==1)
					{
						if((client_addr[ff+1].sin_addr.s_addr=='\0') && (client_addr[ff+1].sin_port=='\0'))
						{
							if(ff!=0)
				                sendto(sockfd, (const char *)buffer, strlen(buffer),MSG_CONFIRM, (const struct sockaddr *) &client_addr[0],c);
								// write acknowledge to next other clients as condition given at starting run time
						}
						else
				            sendto(sockfd, (const char *)buffer, strlen(buffer),MSG_CONFIRM, (const struct sockaddr *) &client_addr[ff+1],c);
							// write acknowledge to next other clients as condition given at starting run time
					}
					else
					{
						for (i = 0; i < max_clients; i++)
						{
							if((client_addr[i].sin_addr.s_addr=='\0') && (client_addr[i].sin_port=='\0'))
							{
								break;
							}
							else
							{
								if(i!=ff)
				                    sendto(sockfd, (const char *)buffer, strlen(buffer),MSG_CONFIRM, (const struct sockaddr *) &client_addr[i],c);
									// write acknowledge to all other clients as condition given at starting run time
							}
						}
					}

                }
    	}

    }

    return 0;
}
