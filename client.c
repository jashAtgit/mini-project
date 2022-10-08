/*===============================================================================

 Author: Battula Jashwanth Sai
 Roll: MT2022029

 CLIENT
 
=================================================================================*/

#include "bank.h"


int intro_menu(){
    system("clear");
    printf("\t\tCLIENT SIDE BANKING\t\t\n");
    printf("1. Create New Account\n2. Login\n3. Exit\n\n\n");

    int ch;
    char buf[MAX_LINE];
    do{
        printf("Enter your choice here : ");
        fgets(buf, MAX_LINE, stdin);

        ch = atoi(buf);
    }while(ch < 1 || ch > 3);
    
    printf("your choice is %d \n", ch);
    return ch;
}

//sock_fd can be made global
void logged_in(){

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
        struct user newu;
        system("clear");
        printf("enter your first name: ");
        fgets(newu.fname, MAX_STR, stdin);
        
        printf("enter your last name: ");
        fgets(newu.lname, MAX_STR, stdin);
        
        printf("choose your username(no spaces): ");        //check for spaces and prompt retry
        fgets(newu.uname, MAX_STR, stdin);

        printf("enter your phone number: ");
        fgets(newu.phone, MAX_STR, stdin);

        strcpy(newu.encrypted,crypt(getpass("create a password:"), SALT));
        printf("encrypted : %s\n", newu.encrypted);
        
        newu.fname[strcspn(newu.fname, "\n")] =  '\0';
        newu.lname[strcspn(newu.lname, "\n")] =  '\0';
        newu.uname[strcspn(newu.uname, "\n")] =  '\0';
        newu.phone[strcspn(newu.phone, "\n")] =  '\0';
        newu.encrypted[strcspn(newu.encrypted, "\n")] = '\0';

        write(sock_fd, newu.fname, sizeof(newu.fname));
        write(sock_fd, newu.lname, sizeof(newu.lname));
        write(sock_fd, newu.uname, sizeof(newu.uname));
        write(sock_fd, newu.phone, sizeof(newu.phone));
        write(sock_fd, newu.encrypted, sizeof(newu.encrypted));

        printf("Account Created Successfully !!!!\n");

        break;
    
    case 2:
        /* get login details from user and send to server to verify */
        ;
        int c=3,ok=0;
        while(c > 0  && !ok){
            struct user dummy;

            system("clear");
            printf("\t\t::::::::::::LOGIN::::::::::::\t\t\n\n");
            
            printf("Enter Username: ");
            fgets(dummy.uname, MAX_STR, stdin);
            

            strcpy(dummy.encrypted,crypt(getpass("Enter Password: "),SALT));
            
            dummy.uname[strcspn(dummy.uname, "\n")] =  '\0';
            dummy.encrypted[strcspn(dummy.encrypted, "\n")] = '\0';

            write(sock_fd, dummy.uname, sizeof(dummy.uname));
            write(sock_fd, dummy.encrypted, sizeof(dummy.encrypted));

            int ok;
            read(sock_fd, &ok, sizeof(ok));
            if(ok){
                printf("Login success\n");
                logged_in();
            }
            else{
                printf("Invalid Credentials ! press enter to try again, %d attemps left \n", --c);
                getc(stdin);
            }
        }
        break;

    default:
        break;
    }
    close(sock_fd);


}