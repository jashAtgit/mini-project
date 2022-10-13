#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <crypt.h>
#include <wait.h>
#include <strings.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_STR 50
#define PASS_LEN 14
#define SALT "ab"
#define SKIP "skip write\n"
#define EXIT "exit\n"
#define CLR_SCREEN "clr\n"


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


