/* Authors: Evangelia Tasiou_02636, Polyxeni Biti_02582
 * 
 * Date: April 1st 2017
 * 
 * Program that imports a directory's files to an encrypted archive file and exports them within a new directory
 * */
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
#include <signal.h>
#include <sys/wait.h>

#define MAGICNO_SIZE 9

int main(int argc, char* argv[]){
    
    pid_t p2, p3, p4, waitpd;
    int fd_file, pdir, parch, pcr, fd_pipe1[2], fd_pipe2[2], pipe1, pipe2, dup_pipe1, dup_pipe2, dup_file, punarch, kill_p, i, status;
    char magicno[MAGICNO_SIZE+1];
    
    if(argc < 5){
        printf("Invalid arguments!\n");
        exit(1);
    }
    
    if(strcmp(argv[1], "-E") == 0){
        fd_file = open(argv[4], O_RDWR | O_CREAT | O_EXCL, 0700);
        if(fd_file < 0){
            perror("Opening file");
            exit(1);
        }
        my_write(fd_file, "P2CRYPTAR", MAGICNO_SIZE, __FILE__, __LINE__);
        pipe1 = pipe(fd_pipe1);
        if(pipe1 < 0){
            perror("Creating pipe1");
            exit(1);
        }
        pipe2 = pipe(fd_pipe2);
        if(pipe2 < 0){
            perror("Creating pipe2");
            exit(1);
        }
        p2 = fork();
        if(p2 < 0){
            perror("Fork p2 problem");
            kill_p = kill(-1, SIGKILL);
            if(kill_p < 0){
                perror("Killing proccesses");
            }
            exit(1);
        }
        if(p2 == 0){
            dup_pipe1 = dup2(fd_pipe1[1], STDOUT_FILENO);
            if(dup_pipe1 < 0){
                perror("Redirecting dirlist output");
                exit(1);
            }
            close(fd_pipe1[0]);
			close(fd_pipe1[1]);
			close(fd_pipe2[0]);
			close(fd_pipe2[1]);
            pdir = execlp("./dirlist", "dirlist", argv[2], NULL);
            if(pdir < 0){
                perror("Executing dirlist");
                exit(1);
            }
        }
        p3 = fork();
        if(p3 < 0){
            perror("Fork p3 problem");
            kill_p = kill(-1, SIGKILL);
            if(kill_p < 0){
                perror("Killing proccesses");
            }
            exit(1);
        }
        if(p3 == 0){
            dup_pipe1 = dup2(fd_pipe1[0], STDIN_FILENO);
            if(dup_pipe1 < 0){
                perror("Redirecting archive input");
                exit(1);
            }
            dup_pipe2 = dup2(fd_pipe2[1], STDOUT_FILENO);
            if(dup_pipe2 < 0){
                perror("Redirecting archive output");
                exit(1);
            }
            close(fd_pipe1[0]);
			close(fd_pipe1[1]);
			close(fd_pipe2[0]);
			close(fd_pipe2[1]);
            parch = execlp("./p2archive", "p2archive", NULL);
            if(parch < 0){
                perror("Executing archive");
                exit(1); 
            }
        }
        p4 = fork();
        if(p4 < 0){
            perror("Fork p4 problem");
            kill_p = kill(-1, SIGKILL);
            if(kill_p < 0){
                perror("Killing proccesses");
            }
            exit(1);
        }
        if(p4 == 0){
            dup_pipe2 = dup2(fd_pipe2[0], STDIN_FILENO);
            if(dup_pipe2 < 0){
                perror("Redirecting crypt input");
                exit(1);
            }
            dup_file = dup2(fd_file, STDOUT_FILENO);
            if(dup_file < 0){
                perror("Redirecting crypt output");
                exit(1);
            }
            close(fd_pipe1[0]);
			close(fd_pipe1[1]);
			close(fd_pipe2[0]);
			close(fd_pipe2[1]);
            pcr = execlp("./p2crypt", "p2crypt", argv[3], NULL);
            if(pcr < 0){
                perror("Executing crypt");
                exit(1);
            }
            close(fd_file);
        }
        close(fd_pipe1[0]);
		close(fd_pipe1[1]);
		close(fd_pipe2[0]);
		close(fd_pipe2[1]);
		
        for(i=0; i<3; i++){
            waitpd = waitpid(-1, &status, 0);
            if(waitpd < 0){
                perror("Waitpid");
                exit(1);
            }
        }
    }
    if(strcmp(argv[1], "-D") == 0){
        fd_file = open(argv[4], O_RDONLY);
        if(errno == ENOENT){
            my_write(STDERR_FILENO, "File does not exist!\n", 21*sizeof(char), __FILE__, __LINE__);
            exit(1);
        }
        if(fd_file < 0){
            perror("Opening encrypted file");
            exit(1);
        }
        my_read(fd_file, magicno, MAGICNO_SIZE, __FILE__, __LINE__);
        magicno[MAGICNO_SIZE] = '\0'; 
        if(strcmp(magicno, "P2CRYPTAR") != 0){
            my_write(STDERR_FILENO, "Wrong magic number!\n", 20*sizeof(char), __FILE__, __LINE__);
            exit(1);
        }
        pipe1 = pipe(fd_pipe1);
        if(pipe1 < 0){
            perror("Creating pipe1");
            exit(1);
        }
        p2 = fork();
        if(p2 < 0){
            perror("Fork p2 problem");
            kill_p = kill(-1, SIGKILL);
            if(kill_p < 0){
                perror("Killing proccesses");
            }
            exit(1);
        }
        if(p2 == 0){
            dup_file = dup2(fd_file, STDIN_FILENO);
            if(dup_file < 0){
                perror("Redirecting crypt input");
                exit(1);
            }
            dup_pipe1 = dup2(fd_pipe1[1], STDOUT_FILENO);
            if(dup_pipe1 < 0){
                perror("Redirecting crypt output");
                exit(1);
            }
            close(fd_pipe1[0]);
            close(fd_pipe1[1]);
            pcr = execlp("./p2crypt", "p2crypt", argv[3], NULL);
            if(pcr < 0){
                perror("Executing crypt");
                exit(1); 
            }
            close(fd_file);
        }
        p3 = fork();
        if(p3 < 0){
            perror("Fork p3 problem");
            kill_p = kill(-1, SIGKILL);
            if(kill_p < 0){
                perror("Killing proccesses");
            }
            exit(1);
        }
        if(p3 == 0){
            dup_pipe1 = dup2(fd_pipe1[0], STDIN_FILENO);
            if(dup_pipe1 < 0){
                perror("Redirecting unarchive input");
                exit(1);
            }
            close(fd_pipe1[1]);
            close(fd_pipe1[0]);
            punarch = execlp("./p2unarchive", "p2unarchive", argv[2], NULL);
            if(punarch < 0){
                perror("Executing unarchive");
                exit(1); 
            }
        }
        close(fd_pipe1[1]);
		close(fd_pipe1[0]);
        for(i=0; i<2; i++){
            waitpd = waitpid(-1, &status, 0);
            if(waitpd < 0){
                perror("Waitpid");
                exit(1);
            }
        }
    }
    return(0);
}
