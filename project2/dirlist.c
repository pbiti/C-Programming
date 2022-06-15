/* Authors: Evangelia Tasiou_02636, Polyxeni Biti_02582
 * 
 * Date: April 1st 2017
 * 
 * Program that finds regular files within a directory
*/

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

int main(int argc, char* argv[]){
    
    DIR *dirp;
    int close_d;
    struct dirent *r_dir;
    char *err_message = "Directory doesn't exist!\n";
    
    if(argc == 1){
        printf("Invalid directory name!\n");
        exit(1);
    }
    dirp = opendir(argv[1]);
    if(errno == ENOENT){
        my_write(STDERR_FILENO, err_message, strlen(err_message), __FILE__, __LINE__);
		exit(1);
    }else{
        if(dirp == NULL){
            perror("opening directory\n");
            dirp = opendir(argv[1]);
			exit(1);
        }
    }
    while((r_dir = readdir(dirp)) != NULL){
        if(r_dir->d_type == DT_REG){
            printf("%s/%s\n", argv[1], r_dir->d_name);
        }
    }
    close_d = closedir(dirp);
    while(close_d < 0){
        perror("closing directory\n");
        close_d = closedir(dirp);
    }
    return(0);
}
