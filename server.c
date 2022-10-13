#include "bank.h"
#include "prompts.h"

#define ADMIN_UNAME "admin"
#define ADMIN_PASS "abWMpd9uBwR.g"
#define USER_FILE "./data/users.bin"
#define ACCOUNTS_FILE "./data/accounts.bin"
#define TRANSACTIONS_FILE "./data/transactions.bin"



#define MAX_LISTEN 5

void clear_screen(int sfd){
    char readBuf[MAX_LINE];

    write(sfd, CLR_SCREEN, sizeof(CLR_SCREEN));
    read(sfd, readBuf, sizeof readBuf); //ack
}

void exit_client(int sfd){
    char readBuf[MAX_LINE];

    write(sfd, EXIT, strlen(EXIT));
    read(sfd, readBuf, sizeof(readBuf));
}

void write_without_read(int sfd, char *msg){
    char readBuf[MAX_LINE];

    write(sfd, SKIP, strlen(SKIP));
    read(sfd, readBuf, sizeof readBuf);  //ready ack
    
    write(sfd, msg, strlen(msg));
    read(sfd, readBuf, sizeof readBuf); // done ack
}


int add_account(int type, int sfd){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    struct account acc;
    acc.type = type;
    acc.active = 1;
    acc.bal = 0;
    //acc.accno = 0;

    struct flock l;

    int fd = open(ACCOUNTS_FILE, O_RDONLY, S_IRWXU);

    if(fd == -1 && errno == ENOENT){
        fd = open(ACCOUNTS_FILE, O_CREAT | O_WRONLY, S_IRWXU);
        acc.accno = 0;
    }
    else if(fd == -1){
        perror("open(userfile)");
        exit(1);
    }
    else{
        off_t offset = lseek(fd, -sizeof(struct account), SEEK_END);

        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = offset;
        l.l_len = sizeof(struct account);
        l.l_pid = getpid();

        fcntl(fd, F_SETLKW, &l);

        struct account prev;

        read(fd, &prev, sizeof(struct account));

        l.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &l);
        close(fd);

        //printf("prev acc no %d\n", prev.accno);
        fd = open(ACCOUNTS_FILE, O_WRONLY, S_IRWXU);
        acc.accno = prev.accno + 1;
    }

    lseek(fd, 0, SEEK_END);
    write(fd, &acc, sizeof(acc));
    close(fd);
    return acc.accno;
}

void add_user(int sfd, int acc_no){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    struct user u;
    u.my_accno = acc_no;
    struct flock l;

    clear_screen(sfd);

    write(sfd, NAME_PROMPT, strlen(NAME_PROMPT));
    bzero(readBuf, sizeof readBuf);
    read(sfd, readBuf, sizeof readBuf);
    sprintf(u.uname, "%s", readBuf);

    write(sfd, AGE_PROMPT, strlen(AGE_PROMPT));
    bzero(readBuf, sizeof readBuf);
    read(sfd, readBuf, sizeof readBuf);
    u.age = atoi(readBuf); 

    write(sfd, SEX_PROMPT, strlen(SEX_PROMPT));
    bzero(readBuf, sizeof readBuf);
    read(sfd, readBuf, sizeof readBuf);
    sprintf(u.sex, "%s", readBuf);

    write(sfd, PASS_PROMPT, strlen(PASS_PROMPT));
    bzero(readBuf, sizeof readBuf);
    read(sfd, readBuf, sizeof readBuf);
    readBuf[strcspn(readBuf, "\n")] = '\0';
    strcpy(u.encrypted,crypt(readBuf, SALT));
    //sprintf(u.encrypted, "%s", readBuf);
    
    int fd = open(USER_FILE, O_RDONLY, S_IRWXU);

    if(fd == -1 && errno == ENOENT){
        printf("first user\n");
        fd = open(USER_FILE, O_CREAT | O_WRONLY, S_IRWXU);
        u.cust_id = 0;
    }
    else if(fd == -1){
        perror("open(userfile)");
        exit(1);
    }
    else{
        off_t offset = lseek(fd, -sizeof(struct user), SEEK_END);

        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = offset;
        l.l_len = sizeof(struct user);
        l.l_pid = getpid();

        fcntl(fd, F_SETLKW, &l);

        struct user prev;

        read(fd, &prev, sizeof(struct user));

        l.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &l);
        close(fd);

        fd = open(USER_FILE, O_WRONLY, S_IRWXU);
        u.cust_id = prev.cust_id + 1;
    }


    lseek(fd, 0, SEEK_END);
    write(fd, &u, sizeof(u));
    close(fd);

    write(sfd, "skip write\n", strlen("skip write\n"));
    read(sfd, readBuf, sizeof(readBuf));
    write(sfd, SUCCESS_USERADDED, strlen(SUCCESS_USERADDED));
    read(sfd, readBuf, sizeof(readBuf));  //read ack 

}

void search_user(int sfd){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    bzero(readBuf, sizeof readBuf);
    bzero(writeBuf, sizeof writeBuf);
    
    clear_screen(sfd);

    write(sfd, SEARCH_INTERFACE, strlen(SEARCH_INTERFACE));
    read(sfd, readBuf, sizeof readBuf);

    struct user u;
    strcpy(u.uname, readBuf);

    struct flock l;

    int fd = open(USER_FILE, O_CREAT | O_RDWR, S_IRWXU);

    if(fd == -1 && errno == ENOENT){
        //no users exits yet
    }
    else if(fd == -1){
        perror("open(search user)");
        exit(1);
    }
    else{

        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = 0;
        l.l_len = 0;
        l.l_pid = getpid();

        fcntl(fd, F_SETLKW, &l);

        struct user curr;
        int rb;
        do{
            rb = read(fd, &curr, sizeof(struct user));
            printf("asked : %s\n", u.uname);
            printf("read : %s\n", curr.uname);
        }while(strcmp(u.uname, curr.uname) != 0 && rb > 0);

        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        

        l.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &l);

        close(fd);
        int active=0;

        //check if user's account is deleted 
        int afd = open(ACCOUNTS_FILE, O_RDONLY, S_IRWXU);
        printf("curr account number %d\n", curr.my_accno);

        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = curr.my_accno * sizeof(struct account);
        l.l_len = sizeof(struct account);
        l.l_pid = getpid();

        fcntl(afd, F_SETLKW, &l);

        struct account a;
        lseek(afd, curr.my_accno * sizeof(struct account), SEEK_SET);
        read(afd, &a, sizeof(struct account));

        l.l_type = F_UNLCK;
        fcntl(afd, F_SETLK, &l);
        close(afd);

        printf("active val %d\n", a.active);
        active = a.active ? 1 : 0;
    
        if(strcmp(u.uname, curr.uname) == 0 && active){
            
            clear_screen(sfd);

            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "User ID : %d", curr.cust_id);
            write_without_read(sfd, writeBuf);

            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "Account ID : %d", curr.my_accno);
            write_without_read(sfd, writeBuf);
            
            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "Username : %s", curr.uname);
            write_without_read(sfd, writeBuf);

            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "Age : %d", curr.age);
            write_without_read(sfd, writeBuf);
            
            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "Sex : %s", curr.sex);
            write_without_read(sfd, writeBuf);
            

            //exit client
            exit_client(sfd);
        }
        
    }
    //send user not exits
    bzero(writeBuf, sizeof writeBuf);
    sprintf(writeBuf, "%s","No account exists with this username..\nexiting....");
    write_without_read(sfd, writeBuf);

    exit_client(sfd);
    
    
}

void delete_account(int sfd){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    bzero(readBuf, sizeof readBuf);
    bzero(writeBuf, sizeof writeBuf);
    
    clear_screen(sfd);

    write(sfd, DELETE_INTERFACE, strlen(DELETE_INTERFACE));
    read(sfd, readBuf, sizeof readBuf);

    struct account a;
    a.accno = atoi(readBuf);
    printf("delete this : %d\n", a.accno);

    struct flock l;

    int afd = open(ACCOUNTS_FILE, O_RDWR, S_IRWXU);

    if(afd == -1){
       perror("open (delete acc)");
    }
    else{
        l.l_type = F_WRLCK;
        l.l_whence = SEEK_SET;
        l.l_start = a.accno * sizeof(struct account);
        l.l_len = sizeof(struct account);
        l.l_pid = getpid();

        fcntl(afd, F_SETLKW, &l);

        struct account curr;
        lseek(afd, a.accno * sizeof(struct account), SEEK_SET);
        read(afd, &curr, sizeof(curr));

        if(curr.active == 1){
            curr.active = 0;
            lseek(afd, -sizeof(struct account), SEEK_CUR);
            write(afd, &curr, sizeof(curr));

            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "%s%d%s", "Account number ", a.accno, " deleted successfully!");
            write_without_read(sfd, writeBuf);
            
        }
        else{
            //already deleted or user does not exists
            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf, "%s", "Account number does not exist..");
            write_without_read(sfd, writeBuf);
        
        }
        l.l_type = F_UNLCK;
        fcntl(afd, F_SETLK, &l);
        close(afd);

        exit_client(sfd);
    }

}

void modify_user(int sfd){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    bzero(readBuf, sizeof readBuf);
    bzero(writeBuf, sizeof writeBuf);
    
    clear_screen(sfd);

    int accno;
    struct user prev;
    struct flock l;

    write(sfd, MODIFY_INTERFACE, strlen(MODIFY_INTERFACE));
    read(sfd, readBuf, sizeof readBuf);
    accno = atoi(readBuf);

    printf("account number to modify : %d\n", accno);

    bzero(readBuf, sizeof readBuf);
    write(sfd, MODIFY_OPTIONS, strlen(MODIFY_OPTIONS));
    read(sfd, readBuf, sizeof readBuf);
    int ch = atoi(readBuf);

    bzero(readBuf, sizeof readBuf);
    write(sfd, PROMPT_MODIFICATION, strlen(PROMPT_MODIFICATION));
    read(sfd, readBuf, sizeof readBuf);
    printf("read buf : %s\n", readBuf);
    char new_val[MAX_STR];
    strcpy(new_val, readBuf);
    printf("new value : %s\n", new_val);

    int ufd = open(USER_FILE, O_RDONLY, S_IRWXU);

    if(ufd == -1){
        perror("open(modify)");
    }
    else{
        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct user);
        l.l_len = sizeof(struct user);
        l.l_pid = getpid();

        fcntl(ufd, F_SETLKW, &l);

        lseek(ufd, accno * sizeof(struct user), SEEK_SET);
        
        read(ufd, &prev, sizeof(prev));

        l.l_type = F_UNLCK;
        fcntl(ufd, F_SETLK, &l);
        close(ufd);

        switch (ch)
        {
        case 1:
            bzero(prev.uname, sizeof(prev.uname));
            strcpy(prev.uname, new_val);
            printf("uname : %s\n", prev.uname);
            break;
        case 2:
            prev.age = atoi(new_val);
            printf("age : %d\n", prev.age);
            break;

        case 3:
            bzero(prev.sex, sizeof(prev.sex));
            strcpy(prev.sex, new_val);
            printf("sex : %s\n", prev.sex);
            break;

        default:
            exit_client(sfd);
            return;
            break;
        }

        ufd = open(USER_FILE, O_WRONLY, S_IRWXU);

        l.l_type = F_WRLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct user);
        l.l_len = sizeof(struct user);
        l.l_pid = getpid();

        fcntl(ufd, F_SETLKW, &l);

        lseek(ufd, accno * sizeof(struct user), SEEK_SET);
        
        write(ufd, &prev, sizeof(prev));

        l.l_type = F_UNLCK;
        fcntl(ufd, F_SETLK, &l);
        close(ufd);

        bzero(writeBuf, sizeof(writeBuf));
        sprintf(writeBuf, "%s", "Account Details Modified Successfully!");
        write_without_read(sfd, writeBuf);

        exit_client(sfd);
    }

}

void process_admin(int sfd){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    clear_screen(sfd);

    write(sfd, ADMIN_INTERFACE, strlen(ADMIN_INTERFACE));
    read(sfd, readBuf, sizeof readBuf);

    int ch = atoi(readBuf);
    if(ch < 1 || ch > 4){
        write(sfd, "exit\n", strlen("exit\n"));
        return;
    }
    switch (ch)
    {
    case 1:
        ;
        int type;

        write(sfd, ACC_TYPE_PROMPT, strlen(ACC_TYPE_PROMPT));
        bzero(readBuf, sizeof readBuf);
        read(sfd, readBuf, sizeof readBuf);
        type = atoi(readBuf);

        int acc_no = add_account(type, sfd);
        printf("returned acc no %d\n", acc_no);

        if(type == 0){
            // normal account
            add_user(sfd, acc_no);

            write(sfd, "exit\n", strlen("exit\n"));
            read(sfd, readBuf, sizeof(readBuf));
        }
        else{
            // joint account
            add_user(sfd, acc_no);
            add_user(sfd, acc_no);

            write(sfd, "exit\n", strlen("exit\n"));
            read(sfd, readBuf, sizeof(readBuf));
        }
            
        break;

    case 2:
        modify_user(sfd);
        break;    

    case 3:         
        delete_account(sfd);
        break;

    case 4:
        search_user(sfd);
        break;
    
    default:
        break;
    }


}

void change_balance(int sfd,  int accno, int amount){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    int afd = open(ACCOUNTS_FILE, O_RDONLY, S_IRWXU);

    struct account a;
    struct flock l;
    struct transaction tran;
    time_t tstamp;

    tran.accno = accno;

    long int old_bal, new_bal;

    if(afd == -1){
        perror("open(change_bal)");
    }
    else{
        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct account);
        l.l_len = sizeof(struct account);
        l.l_pid = getpid();

        fcntl(afd, F_SETLKW, &l);
        lseek(afd, accno * sizeof(struct account), SEEK_SET);

        read(afd, &a, sizeof(a));

        l.l_type = F_UNLCK;
        fcntl(afd, F_SETLK, &l);
        close(afd);

        old_bal = a.bal;

        if(amount > 0){
            //deposit
            new_bal = old_bal + amount;
        }
        else{
            //withdraw
            if(old_bal + amount < 0){
                //not enuf money
                //write and return;
                bzero(writeBuf, sizeof(writeBuf));
                sprintf(writeBuf, "%s", "Not enough money in the account to process request !!\nexiting...");
                write_without_read(sfd, writeBuf);

                exit_client(sfd);
                return;
            }
            else{
                new_bal = old_bal + amount;
            }
        }
        a.bal = new_bal;
        //take lock and write new bal
        afd = open(ACCOUNTS_FILE, O_WRONLY, S_IRWXU);
        
        l.l_type = F_WRLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct account);
        l.l_len = sizeof(struct account);
        l.l_pid = getpid();

        fcntl(afd, F_SETLKW, &l);
        lseek(afd, accno * sizeof(struct account), SEEK_SET);

        tstamp = time(NULL);
        
        write(afd, &a, sizeof(a));

        l.l_type = F_UNLCK;
        fcntl(afd, F_SETLK, &l);
        close(afd);
        
        int tfd = open(TRANSACTIONS_FILE, O_APPEND | O_WRONLY, S_IRWXU);

        if(tfd == -1 && errno == ENOENT){
            tfd = open(TRANSACTIONS_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
        }
        tran.old_val = old_bal;
        tran.new_val = new_bal;

        strcpy(tran.transaction_time, ctime(&tstamp));

        l.l_type = F_WRLCK;
        l.l_whence = SEEK_END;
        l.l_start = 0;
        l.l_len = sizeof(struct transaction);
        l.l_pid = getpid();

        fcntl(tfd, F_SETLKW, &l);
        lseek(tfd, 0, SEEK_END);

        write(tfd, &tran, sizeof(tran));

        l.l_type = F_UNLCK;
        fcntl(tfd, F_SETLK, &l);
        close(tfd);
        
        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf, "%s", "Transaction Successfull !!");
        write_without_read(sfd, writeBuf);

        exit_client(sfd);

    }
}

void balance_enquiry(int sfd, int accno){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    int afd = open(ACCOUNTS_FILE, O_RDONLY, S_IRWXU);

    struct account a;
    struct flock l;

    if(afd == -1){
        perror("open(change_bal)");
    }
    else{
        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct account);
        l.l_len = sizeof(struct account);
        l.l_pid = getpid();

        fcntl(afd, F_SETLKW, &l);
        lseek(afd, accno * sizeof(struct account), SEEK_SET);

        read(afd, &a, sizeof(a));

        l.l_type = F_UNLCK;
        fcntl(afd, F_SETLK, &l);
        close(afd);

        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s%ld%s", "Your Current Balance : ", a.bal, " INR");
        write_without_read(sfd, writeBuf);

        exit_client(sfd);
    }
}

void change_pass(int sfd, int accno){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    bzero(writeBuf, sizeof writeBuf);
    sprintf(writeBuf, "%s", "Enter New Password: ");
    write(sfd, &writeBuf, strlen(writeBuf));
    bzero(readBuf, sizeof readBuf);
    read(sfd, &readBuf, sizeof(readBuf));

    struct user prev;
    struct flock l;

    int ufd = open(USER_FILE, O_RDONLY, S_IRWXU);

    if(ufd == -1){
        perror("open (change pass)");
    }
    else{
        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct user);
        l.l_len = sizeof(struct user);
        l.l_pid = getpid();

        fcntl(ufd, F_SETLKW, &l);

        lseek(ufd, accno * sizeof(struct user), SEEK_SET);
        
        read(ufd, &prev, sizeof(prev));

        l.l_type = F_UNLCK;
        fcntl(ufd, F_SETLK, &l);
        close(ufd);
        
        char pass[PASS_LEN];
        strcpy(pass, readBuf);

        bzero(prev.encrypted, sizeof(prev.encrypted));
        strcpy(prev.encrypted, crypt(pass, SALT));

        ufd = open(USER_FILE, O_WRONLY, S_IRWXU);

        l.l_type = F_WRLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct user);
        l.l_len = sizeof(struct user);
        l.l_pid = getpid();

        fcntl(ufd, F_SETLKW, &l);

        lseek(ufd, accno * sizeof(struct user), SEEK_SET);
        
        write(ufd, &prev, sizeof(prev));

        l.l_type = F_UNLCK;
        fcntl(ufd, F_SETLK, &l);
        close(ufd);

        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s", "Password Change Successfull !!");
        write_without_read(sfd, writeBuf);

        exit_client(sfd);

    }
}

void user_view_details(int sfd, int accno){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    clear_screen(sfd);

    struct user u;
    struct transaction tran;
    struct flock l;

    int tfd,c=1;
    
    int ufd = open(USER_FILE, O_RDONLY, S_IRWXU);

    if(ufd == -1){
        perror("open(user view detatils)");
        exit(1);
    }
    else{

        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = accno * sizeof(struct user);
        l.l_len = sizeof(struct user);
        l.l_pid = getpid();

        fcntl(ufd, F_SETLKW, &l);

        lseek(ufd, accno * sizeof(struct user), SEEK_SET);
        read(ufd, &u, sizeof(u));

        l.l_type = F_UNLCK;
        fcntl(ufd, F_SETLK, &l);
        close(ufd);

        //send user details to client
        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s%d", "Customer ID : ", u.cust_id);
        write_without_read(sfd, writeBuf);

        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s%d", "Account Number : ", u.my_accno);
        write_without_read(sfd, writeBuf);

        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s%s", "Username : ", u.uname);
        write_without_read(sfd, writeBuf);

        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s%d", "Age : ", u.age);
        write_without_read(sfd, writeBuf);

        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s%s", "Sex : ", u.sex);
        write_without_read(sfd, writeBuf);


        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf,"%s", "\n||||| Previous Transactions |||||");
        write_without_read(sfd, writeBuf);

        tfd = open(TRANSACTIONS_FILE, O_RDONLY, S_IRWXU);

        l.l_type = F_RDLCK;
        l.l_whence = SEEK_SET;
        l.l_start = 0;
        l.l_len = 0;
        l.l_pid = getpid();

        fcntl(tfd, F_SETLKW, &l);

        lseek(tfd, 0, SEEK_SET);
        int rb;
        while(read(tfd, &tran, sizeof(tran))){
            if(tran.accno == accno){
                //send transactions details to client
                bzero(writeBuf, sizeof writeBuf);
                sprintf(writeBuf,"%s%d", "Transaction No: ", c++);
                write_without_read(sfd, writeBuf);

                bzero(writeBuf, sizeof writeBuf);
                sprintf(writeBuf,"%s%ld", "Old Balance : ", tran.old_val);
                write_without_read(sfd, writeBuf);

                bzero(writeBuf, sizeof writeBuf);
                sprintf(writeBuf,"%s%ld", "New Balance : ", tran.new_val);
                write_without_read(sfd, writeBuf);

                bzero(writeBuf, sizeof writeBuf);
                sprintf(writeBuf,"%s%s", "Timestamp : ", tran.transaction_time);
                write_without_read(sfd, writeBuf);
            }
        }
        if(c == 1){
            bzero(writeBuf, sizeof writeBuf);
            sprintf(writeBuf,"%s", "None so far");
            write_without_read(sfd, writeBuf);
        }
        

        l.l_type = F_UNLCK;
        fcntl(tfd, F_SETLK, &l);
        close(tfd);

        exit_client(sfd);
    }
}

void process_user(int sfd, int accno){
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    bzero(readBuf, sizeof readBuf);
    bzero(writeBuf, sizeof writeBuf);
    
    clear_screen(sfd);

    int ch, amount;

    bzero(writeBuf, sizeof writeBuf);
    sprintf(writeBuf, "%s", USER_INTERFACE);
    write(sfd, &writeBuf, strlen(writeBuf));
    bzero(readBuf, sizeof readBuf);
    read(sfd, &readBuf, sizeof(readBuf));
    ch = atoi(readBuf);

    switch (ch)
    {
    case 1:
        //deposit
        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf, "%s", "Enter Deposit Amount : ");
        write(sfd, &writeBuf, strlen(writeBuf));
        bzero(readBuf, sizeof readBuf);
        read(sfd, &readBuf, sizeof(readBuf));
        amount = atoi(readBuf);

        change_balance(sfd, accno, amount);
        break;

    case 2:
        // withdraw
        bzero(writeBuf, sizeof writeBuf);
        sprintf(writeBuf, "%s", "Enter Withdraw Amount : ");
        write(sfd, &writeBuf, strlen(writeBuf));
        bzero(readBuf, sizeof readBuf);
        read(sfd, &readBuf, sizeof(readBuf));
        amount = atoi(readBuf);

        change_balance(sfd, accno, -amount);
        break;

    case 3:
        balance_enquiry(sfd, accno);
        break;

    case 4:
        change_pass(sfd, accno);
        break;

    case 5:
        user_view_details(sfd, accno);
        break;

    default:
        break;
    }

}

void login(int sfd){

    int isAdmin;
    char readBuf[MAX_LINE], writeBuf[MAX_LINE];

    bzero(readBuf, sizeof readBuf);
    bzero(writeBuf, sizeof writeBuf);

    char uname[MAX_STR];
    char pass[PASS_LEN];

    read(sfd, uname, MAX_STR);
    read(sfd, pass, PASS_LEN);

    isAdmin = ((strcmp(uname, ADMIN_UNAME) == 0 ) && ( strcmp(pass, ADMIN_PASS) == 0 )) ? 1 : 0;

    write(sfd, &isAdmin, sizeof(isAdmin));

    if(isAdmin){
        process_admin(sfd);
    }
    else{
        int ufd = open(USER_FILE, O_RDONLY, S_IRWXU);

        if(ufd == -1){
            perror("open(login)");
        }
        else{
            struct user curr;
            struct flock l;

            l.l_type = F_RDLCK;
            l.l_whence = SEEK_SET;
            l.l_start = 0;
            l.l_len = 0;
            l.l_pid = getpid();

            fcntl(ufd, F_SETLKW, &l);
            lseek(ufd, 0, SEEK_SET);
            int rb;
            do{
                rb = read(ufd, &curr, sizeof(curr));
            }while(strcmp(curr.uname, uname) != 0 && rb > 0);

            if(strcmp(curr.uname, uname) == 0){
                //check password
                int ok = strcmp(curr.encrypted, pass) == 0 ? 1 : 0;
                if(ok){
                    process_user(sfd, curr.my_accno);
                }
                else{
                    //invalid creds
                    bzero(writeBuf, sizeof writeBuf);
                    sprintf(writeBuf, "%s", "Invalid Credentials !!");
                    write_without_read(sfd,writeBuf);

                    exit_client(sfd);
                }
            }
            else{
                //user doesnt exists
                bzero(writeBuf, sizeof writeBuf);
                sprintf(writeBuf, "%s", "No user exists with the given username");
                write_without_read(sfd, writeBuf);

                exit_client(sfd);
            }
        }
        //return;
    }
}

void new_session(int sfd){
    login(sfd);
}

void main(){

    //setbuf(stdout, NULL);   //uncomment to set stdout to unbuffered

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
        pid_t p = fork();
        if(p == -1){
            perror("fork");
        }
        if(p == 0){
            // child
            new_session(new_fd);
            close(new_fd);
        }
    }
    close(sock_fd);
}