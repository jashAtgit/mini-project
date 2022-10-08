/*===============================================================================

 Author: Battula Jashwanth Sai
 Roll: MT2022029
 
 SERVER

=================================================================================*/

#include "bank.h"

#define MAX_LISTEN 5

void *process_client(void * arg){
    int sfd = *(int *)arg;
    printf("socked id is %d \n",sfd);

    int ch;
    if(read(sfd, &ch, sizeof(ch)) < 0){
        perror("read");
        exit(1);
    }
    
    char data[MAX_STR];
    FILE * fp = fopen("./data/details.txt", "a+");

    switch (ch)
    {
    case 1:
        //create account
        //wait to read details and store in file(using mutex)
        ;
        
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

        if(fp == NULL){
            perror("open");
            pthread_exit(NULL);
        }
        pthread_mutex_lock(&mtx);
        /* ================================ start of critical section ================================ */
        fseek(fp, 0, SEEK_END);
        fprintf(fp,"%d;;", random_number(100000, 999999+1));    //assign a 6 digit account number to the new account

        int readBytes;
        while((readBytes = read(sfd, data, MAX_STR)) > 0) {
            printf("data: %s\n", data);
            printf("output fprintf %d\n",fprintf(fp, "%s;;",data));
        }

        // add delimiter here
        fprintf(fp, "\n");

        /* ================================ end of critical section ==================================*/
        pthread_mutex_unlock(&mtx);

        break;
    case 2:
        /* verify login details recvd from client */
        ;
        int c=3,ok=0;
        while(c > 0 && !ok){
            struct user u, u2; 
            read(sfd, u.uname, sizeof(u.uname));
            read(sfd, u.encrypted, sizeof(u.encrypted));

            fseek(fp, 0, SEEK_SET);

            int status;
            char newline[MAX_STR];
            while((status = fscanf(fp, "%d;;%[^;;];;%[^;;];;%[^;;];;%[^;;];;%[^;;]",&u2.acc_no, u2.fname, u2.lname, u2.uname, u2.phone, u2.encrypted)) && status == 6){
                fscanf(fp,"%[^\n]",newline);
                int found = strcmp(u2.uname, u.uname) == 0;
                if(found){
                    break;
                }
            }
            int ok = strcmp(u2.encrypted, u.encrypted) == 0;
            
            //sending login status to client
            write(sfd, &ok, sizeof(ok));
            c--;
        }

        break;

    default:
        break;
    }
    fclose(fp);
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