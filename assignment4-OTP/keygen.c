#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>

int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Too many argument. Syntax should be keygen 'keylength'\n");
        exit(1);
    }

    srand(time(0));         // generate random number
    
    int i=0;
    int num=0;
    num = atoi(argv[1]);        // get length

    int temp=0;
    char key[num];              // create string of size length
    for (i=0;i < num; i++){
        temp = (rand() % 27) + 64;      // create random number
        if (temp == 64){                // if last character
            temp = 32;                  // make it a sace
        }
        key[i] = (char) temp;           // put it to string
    }

    for (i=0;i < num; i++){
        printf("%c", key[i]);           // print string (key)
    }
    printf("\n");
    return 0;
}
