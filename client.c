#include "bank.h"


void process_requests(int sock_fd){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    int r;
    do{
        bzero(readBuf, sizeof readBuf);
        bzero(writeBuf, sizeof writeBuf);

        //printf("waiting to read..\n");
        r = read(sock_fd, readBuf, MAX_LINE);
        
        if(strcmp(readBuf , CLR_SCREEN) == 0){
            system("clear");
            write(sock_fd, "1", 1); 
            continue;
        }

        if(strcmp(readBuf, SKIP) == 0){
            write(sock_fd, "1", 1);
            bzero(readBuf, sizeof readBuf);
            read(sock_fd, readBuf, MAX_LINE);
            printf("%s\n", readBuf);
            write(sock_fd, "1", 1);
            continue;
        }
        
        if(strcmp(readBuf, EXIT) == 0){
            write(sock_fd, "1", 1);
            exit(1);
        }

        printf("%s\n", readBuf);

        fgets(writeBuf, sizeof writeBuf, stdin);
        if (strlen(writeBuf) > 0 && writeBuf[strlen(writeBuf) - 1] == '\n')
            writeBuf[strlen(writeBuf) - 1] = '\0';
        write(sock_fd, writeBuf, strlen(writeBuf));
        
    }while(r > 0);

}


void log_in(int sock_fd){
    char uname[MAX_STR];
    char pass[PASS_LEN];

    printf("Enter Username: ");
    fgets(uname, sizeof(uname), stdin);

    strcpy(pass,crypt(getpass("Password: "), SALT));

    uname[strcspn(uname, "\n")] = '\0';
    pass[strcspn(pass,"\n")] = '\0';

    printf("user : %s pass %s \n", uname, pass);

    write(sock_fd, uname, sizeof(uname));
    write(sock_fd, pass, sizeof(pass));
    
    int isAdmin;
    read(sock_fd, &isAdmin, sizeof(isAdmin));
    printf("isAdmin %d\n", isAdmin);
    if(isAdmin){
        printf("Logged in as admin...\n");
    }
    else{
        //read and check if user
        printf("Logged in as user...\n");
    }
    
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

    log_in(sock_fd);
    process_requests(sock_fd);

    close(sock_fd);


}