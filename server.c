/*===============================================================================

 Author: Battula Jashwanth Sai
 Roll: MT2022029
 
 SERVER

=================================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>

#define MAX_LISTEN 5
#define LINE_BUF 1024

void *process_client(void * arg){
    int sfd = *(int *)arg;
    printf("socked id is %d \n",sfd);

    int ch;
    if(read(sfd, &ch, sizeof(ch)) < 0){
        perror("read");
        exit(1);
    }
    
    switch (ch)
    {
    case 1:
        //create account
        //wait to read details and store in file(use file locking)
        ;
        char name[LINE_BUF], uname[LINE_BUF];
        read(sfd, &name, sizeof(name));
        read(sfd, &uname, sizeof(uname));

        printf("name : %s", name);
        printf("username : %s", uname);

        break;
    
    default:
        break;
    }
    //printf("data recvd : %d \n", ch);
    close(sfd);

}

void main(){
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server.sin_port = htons(2222);

    
    if(bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1){
        perror("bind");
        exit(1);
    }

    if(listen(sock_fd, MAX_LISTEN) == -1){
        perror("listen");
        exit(1);
    }

    int new_fd;
    socklen_t cli_addrlen = sizeof(client);
    while(1){
        new_fd = accept(sock_fd, (struct sockaddr *)&client, &cli_addrlen);

        if(new_fd == -1){
            perror("accept");
            exit(1);
        }

        pthread_t t;
        if(pthread_create(&t, NULL, process_client, (void *)&new_fd) != 0){
            perror("thread create");
            exit(1);
        }
        pthread_join(t, NULL);
    }
    close(sock_fd);
}