#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

ssize_t my_read(int fd, void* buf, size_t count, char* file, int line){
    ssize_t rd, rd_data = 0;
    
    rd = read(fd, buf, count);
    if(rd < 0){
        perror("Reading input data");
        printf("Problem at %d line of %s file\n", line, file);
        exit(1);
    }
    
    while(rd  < count){
        rd_data = read(fd, buf + rd , count - rd);
        if(rd_data < 0){
            perror("Reading input data");
            printf("Problem at %d line of %s file\n", line,file);
            exit(1);
        }
        if(rd_data == 0){
            return(rd);
        }
        rd = rd + rd_data;
    }
    return(rd);
}

ssize_t my_write(int fd, const void *buf, size_t count, char *file, int line){
    ssize_t wr, wr_data = 0;
    
    wr = write(fd, buf, count);
    if(wr < 0){
        perror("Writing data");
        printf("Problem at %d line of %s file\n", line, file);
        exit(1);
    }
    while(wr < count){
        wr_data = write(fd, buf + wr, count - wr );
        if(wr_data < 0){
            perror("Writing data");
            printf("Problem at %d line of %s file\n", line,file);
            exit(1);
        }
        if(wr_data == 0){
            return(wr);
        }
        wr = wr + wr_data;
    }
    return(wr);
}
