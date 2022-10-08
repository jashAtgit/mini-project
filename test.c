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

int random_number(int low_num, int hi_num){
    /* generate random numbers between [low_num, hi_num) */
    srand(time(NULL));
    int result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

void main(){
    char encrypted[14], test[14];
    char salt[] = "ab";
    strcpy(encrypted, crypt("hello", salt));
    printf("encrypted : %s\n", encrypted);

    strcpy(test, crypt("hellos", salt));
    printf("test : %s\n", test);
    if(strcmp(encrypted, test) == 0){
        printf("access granted\n");
    }
    else{
        printf("access denied\n");
    }

}