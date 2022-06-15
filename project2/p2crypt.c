/* Authors: Evangelia Tasiou_02636, Polyxeni Biti_02582
 * 
 * Date: April 1st 2017
 * 
 * Program that encrypts or decrypts a file with a given key
 */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "helper.h"

int main(int argc, char* argv[]){
    char byte_read[strlen(argv[1])];
    int i;
    ssize_t rd, wr;
    
    if(argc == 1){
        printf("Invalid key!\n");
        exit(1);
    }
    
    while(1){
        rd = my_read(STDIN_FILENO, byte_read, strlen(argv[1])*sizeof(char), __FILE__, __LINE__);
        
        if(rd == 0){ break;}
        for(i=0; i<strlen(argv[1]); i++){
            byte_read[i] = (byte_read[i]^argv[1][i]);
        }
        wr = my_write(STDOUT_FILENO, byte_read, rd, __FILE__, __LINE__);
        while(wr < rd){
            wr = my_write(STDOUT_FILENO, byte_read + wr, rd - wr, __FILE__, __LINE__);
        }
    }
    return(0);
}
