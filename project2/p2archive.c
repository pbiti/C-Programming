/* Authors: Evangelia Tasiou_02636, Polyxeni Biti_02582
 * 
 * Date: April 1st 2017
 * 
 * Program that archives regular files
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

#define MAX_SIZE 250

int main(int argc, char* argv[]){
    
    char buffer[250] = "a" , format[20];
    char *file_name, *data;
    int i, name_size, stat1, fd, count = 0;
    ssize_t wr_data;
    ssize_t rd_data = 5;
    struct stat info;
    
    data = (char*)malloc(512*sizeof(char));
    if(data == NULL){
        printf("Memory problem!\n");
        exit(1);
    }
    
    while(1){
        for(i=0; i<MAX_SIZE; i++){
            buffer[i] = '\0';
        }
        sprintf(format, "%%%ds", MAX_SIZE - 1); 
        scanf(format, buffer);
        if(buffer[0] != '\0'){
            file_name = strrchr(buffer, '/');
            name_size = strlen(file_name) - 1;
            my_write(STDOUT_FILENO, &name_size, sizeof(int), __FILE__, __LINE__);
        
            my_write(STDOUT_FILENO, file_name + 1, strlen(file_name) - 1, __FILE__, __LINE__);
            
            stat1 = stat(buffer, &info);
            if(stat1 < 0){
                perror("Getting info from stat");
               exit(1);
            }
            my_write(STDOUT_FILENO, &(info.st_atime), sizeof(time_t), __FILE__, __LINE__);
            
            my_write(STDOUT_FILENO, &(info.st_mtime), sizeof(time_t), __FILE__, __LINE__);
           
            my_write(STDOUT_FILENO, &(info.st_mode), sizeof(mode_t), __FILE__, __LINE__);
            
            my_write(STDOUT_FILENO, &(info.st_size), sizeof(off_t), __FILE__, __LINE__);
           
            fd = open(buffer, O_RDONLY);
            while(fd < 0){
                perror("Opening file to read data");
                fd = open(buffer, O_RDONLY);
            }
            if(info.st_size < 512){
                rd_data = my_read(fd, data, info.st_size, __FILE__, __LINE__);
               
                wr_data = my_write(STDOUT_FILENO, data, rd_data, __FILE__, __LINE__);
                if(wr_data < rd_data){
                    wr_data = my_write(STDOUT_FILENO, data + wr_data, rd_data - wr_data, __FILE__, __LINE__);
                }
            }
            while(rd_data != 0 && (info.st_size >= 512)){
                if(info.st_size - count < 512){
                    rd_data = my_read(fd, data, info.st_size - count, __FILE__, __LINE__);
                   
                    wr_data = my_write(STDOUT_FILENO, data, rd_data, __FILE__, __LINE__);
                    if(wr_data < rd_data){
                        wr_data = my_write(STDOUT_FILENO, data + wr_data, rd_data - wr_data, __FILE__, __LINE__);
                    }
                    break;
                }
                rd_data = my_read(fd, data, 512, __FILE__, __LINE__);
                
                wr_data = my_write(STDOUT_FILENO, data, rd_data, __FILE__, __LINE__);
                if(wr_data < rd_data){
                    wr_data = my_write(STDOUT_FILENO, data + wr_data, rd_data - wr_data, __FILE__, __LINE__);
                }
                count = count + rd_data;
            }
            count = 0;
        }else{ break;}
    }
    free(data);
    
 return(0);   
}
