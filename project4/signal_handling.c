/* Authors: Polyxeni Biti_02582, Evangelia Tasiou_02636 
 * 
 * Date: 27/05/2018
 * 
 * Program that prints the value of a counter every 5 secs*/
#include<stdio.h>
#include<errno.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[]){
    sigset_t set, sig_pending;
    int counter = 0, sig_check, max_count = atoi(argv[2]), block = atoi(argv[4]);
    pid_t pid;
    struct sigaction act = {{0}};
    
    void handler(int signo){
    if(signo == SIGUSR1){
        printf("Caught signal\n");
        counter = 0;
    }
    return;
}
    if(argc < 5){
       fprintf(stderr, "Invalid Arguments!\n");
       exit(1);
    }
    
    pid = getpid();
    sigemptyset(&set); //clears variable set
    sigaddset(&set, SIGUSR1); 
    act.sa_handler = handler;
    
    if(block == 1){
        
        sig_check = sigprocmask(SIG_BLOCK, &set, NULL); //blocking SIGUSR1 for the first half of the repeats
        if(sig_check < 0){
            perror("blocking signal 1");
        }
        while(counter < (max_count/2)){
            counter++;
            printf("pid is : %d | counter : %d/%d\n", pid, counter, max_count);
            sleep(5); 
        }
        
        sigemptyset(&sig_pending);
        sigpending(&sig_pending);
        
        if(sigismember(&sig_pending, SIGUSR1)){
            act.sa_handler = handler;
            sigaction(SIGUSR1, &act, NULL);
        }
        
        sig_check = sigprocmask(SIG_UNBLOCK, &set, NULL);
        if(sig_check < 0){
            perror("unblocking signal 1");
        }
        
        while(counter < max_count){
            counter++;
            printf("pid is : %d | counter : %d/%d\n", pid, counter, max_count);
            sleep(5);
           
            sigaction(SIGUSR1, &act, NULL);
        }
        
    }else{ //if block == 0
        sig_check = sigprocmask(SIG_UNBLOCK, &set, NULL);
        if(sig_check < 0){
            perror("unblocking signal 0");
        }
        while(counter < max_count){
            counter++;
            printf("pid is : %d | counter : %d/%d\n", pid, counter, max_count);
            sleep(5);
            
            sigaction(SIGUSR1, &act, NULL);
        }
    }
   return 0;
}
