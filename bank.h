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

#define MAX_LINE 1024
#define MAX_STR 50
#define PASS_LEN 14
#define SALT "ab"

struct user{
    int acc_no;
    char fname[MAX_STR];
    char lname[MAX_STR];
    char uname[MAX_STR];
    char phone[MAX_STR];
    char encrypted[PASS_LEN];
};

int random_number(int low_num, int hi_num){
    /* generate random numbers between [low_num, hi_num) */
    srand(time(NULL));
    int result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}