/* Authors: Evangelia Tasiou_02636, Polyxeni Biti_02582
 * 
 * Date: April 1st 2017
 * 
 * Program that unarchives an archived file's data within a new directory
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
#include <sys/time.h>
#include <utime.h>
#include "helper.h"

#define MAX_SIZE 250
int main(int argc, char* argv[]){
    
    struct stat check_dir;
    int make_dir, name_len, fd, count = 0, close_f, i, ch_dir, u_times, ch_mod;
    ssize_t wr_data;
    ssize_t rd_data = 5;
    char name[MAX_SIZE] = {'\0'};
    struct stat info;
    char *data;
    struct timeval time[2];
    
    if(stat(argv[1], &check_dir) == -1){
        make_dir = mkdir(argv[1], 0700);
        if(make_dir < 0){
            perror("Creating new directory\n");
            exit(1);
        }
    }else{
       my_write(STDERR_FILENO, "Directory already exists!\n", 26, __FILE__, __LINE__);
    }
    ch_dir = chdir(argv[1]);
    while(ch_dir < 0){
        perror("Changing current directory\n");
        ch_dir = chdir(argv[1]);
    }
    data = (char*)malloc(512 * sizeof(char));
    if(data == NULL){
        printf("Memory problem!\n");
        exit(1);
    }
    while(1){
        my_read(STDIN_FILENO, &name_len, sizeof(int), __FILE__, __LINE__);
        
        my_read(STDIN_FILENO, name, name_len, __FILE__, __LINE__);
        if(name[0] == '\0'){
            exit(1);
        }
        fd = open(name, O_RDWR | O_CREAT, 0700);
        if(fd < 0){
            perror("Creating new file in dir\n");
            exit(1);
        }
       
        my_read(STDIN_FILENO, &(info.st_atime), sizeof(time_t), __FILE__, __LINE__);
        
        my_read(STDIN_FILENO, &(info.st_mtime), sizeof(time_t), __FILE__, __LINE__);
        
        my_read(STDIN_FILENO, &(info.st_mode), sizeof(mode_t), __FILE__, __LINE__);
        
        my_read(STDIN_FILENO, &(info.st_size), sizeof(off_t), __FILE__, __LINE__);
       
        if(info.st_size < 512){
            rd_data = my_read(STDIN_FILENO, data, (ssize_t)info.st_size, __FILE__, __LINE__);
            
            wr_data = my_write(fd, data, rd_data, __FILE__, __LINE__);
            while(wr_data < rd_data){
                wr_data = my_write(fd, data + wr_data, rd_data - wr_data, __FILE__, __LINE__);
            }
        }
        while(count < info.st_size && info.st_size >= 512 && rd_data != 0){
            if((info.st_size - count) < 512){
                rd_data = my_read(STDIN_FILENO, data, (ssize_t)info.st_size - count, __FILE__, __LINE__);
    
                wr_data = my_write(fd, data, rd_data, __FILE__, __LINE__);
                while(wr_data < rd_data){
                    wr_data = my_write(fd, data + wr_data, rd_data - wr_data, __FILE__, __LINE__);
                }
                break;
            }else{
                rd_data = my_read(STDIN_FILENO, data, 512, __FILE__, __LINE__);
            }
            wr_data = my_write(fd, data, rd_data, __FILE__, __LINE__);
            while(wr_data < rd_data){
                wr_data = my_write(fd, data + wr_data, rd_data - wr_data, __FILE__, __LINE__);
            }
            count = count+ rd_data;
        }
        count = 0;
        time[0].tv_usec = 0;
        time[0].tv_sec = info.st_atime;
        
        time[1].tv_usec = 0;
        time[1].tv_sec = info.st_mtime;
        
        u_times = utimes(name, time);
        if(u_times < 0){
            perror("Utimes problem");
        }
        ch_mod = chmod(name, info.st_mode);
        if(ch_mod < 0){
            perror("Chmod problem\n");
            exit(1);
        }
        close_f = close(fd);
        if(close_f < 0){
            perror("Closing file\n");
            exit(1);
        }
        for(i = 0; i<MAX_SIZE; i++){ //preparing name[] to store the next name
            name[i] = '\0';
        }
    }
    free(data);
    return(0);
}
