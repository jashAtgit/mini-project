/*===============================================================================

 Author: Battula Jashwanth Sai
 Roll: MT2022029

 CLIENT
 
=================================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define LINE_BUF 1024


int intro_menu(){
    printf("\t\tCLIENT SIDE BANKING\t\t\n");
    printf("1. Create New Account\n2. Login\n3. Exit\n\n\n");

    int ch;
    char buf[LINE_BUF];
    do{
        printf("Enter your choice here : ");
        fgets(buf, LINE_BUF, stdin);

        ch = atoi(buf);
    }while(ch < 1 || ch > 3);
    
    printf("your choice is %d \n", ch);
    return ch;
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

    if(connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1){
        perror("connect");
        exit(1);
    }

    int ch = intro_menu();

    write(sock_fd, &ch, sizeof(ch));
    switch (ch)
    {
    case 1:
        /* create account */
        //input details and send to server
        ;
        char name[LINE_BUF], uname[LINE_BUF];
        printf("Enter Name: ");
        fgets(name, LINE_BUF, stdin);
        printf("enter username: ");
        fgets(uname, LINE_BUF, stdin);

        write(sock_fd, &name, sizeof(name));
        write(sock_fd, &uname, sizeof(uname));
        break;
    
    default:
        break;
    }
    close(sock_fd);


}