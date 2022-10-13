/*===============================================================================

 Author: Battula Jashwanth Sai
 Roll: MT2022029
 
=================================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <crypt.h>
#include <time.h>

#define MAX_STRING 50
#define MAX_STR 50
#define PASS_LEN 14

#define USER_FILE "./data/users.bin"
#define ACCOUNTS_FILE "./data/accounts.bin"
#define TRANSACTIONS_FILE "./data/transactions.bin"


int random_number(int low_num, int hi_num){
    /* generate random numbers between [low_num, hi_num) */
    srand(time(NULL));
    int result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

struct transaction{
    int accno;
    long int old_val;
    long int new_val;
    //time_t transaction_time;
    char transaction_time[100];
};

struct account{
    int accno;
    int type;               // 0 for normal account , 1 for joint
    long int bal;
    int active;             // 1 for active, 0 for inactive
};

struct user{
    int cust_id;              
    int my_accno;
    char uname[MAX_STR];           //associated account number
    int age;
    char sex[MAX_STR];            //"male" or "female"
    char encrypted[PASS_LEN];
};

void main(){
    
    int ufd = open(USER_FILE,O_RDWR, S_IRWXU);
    int afd = open(ACCOUNTS_FILE,O_RDWR, S_IRWXU);
    int tfd = open(TRANSACTIONS_FILE,O_RDWR, S_IRWXU);


    struct user u;
    struct account a;
    struct transaction t;

    while(read(tfd, &t, sizeof(struct transaction))){
        printf("accno : %d\nold val : %ld\nnew val: %ld\ntimestamp: %s\n", t.accno, t.old_val,t.new_val, t.transaction_time);
    }

    while(read(ufd, &u, sizeof(struct user))){
        printf("cid : %d\naccno :%d\nuname: %s\nage : %d\nsex : %s\npass : %s\n", u.cust_id, u.my_accno, u.uname, u.age, u.sex, u.encrypted);
    }
    printf("\n\n");
    while(read(afd, &a, sizeof(a))){
        printf("accno : %d\ntype : %d\nbal : %ld\nactive :%d\n", a.accno, a.type, a.bal, a.active);
    }   

    close(tfd);
    close(ufd);
    close(afd);
    
   

   

   
}