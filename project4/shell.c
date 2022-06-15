/* Authors: Polyxeni Biti_02582, Evangelia Tasiou_02636 
 * 
 * Date: 27/05/2018
 * 
 * Program that resembles the shell */
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<errno.h>
#include<poll.h>

#define MAX_CHAR 5
#define NAME_SIZE 100

struct node{
    pid_t pid;
    int args_num;
    char **orismata;
    int running; //flag that equals to 1 if the process is running and 0 if it's not
    struct node *next;
    struct node *prev;
};

struct node *head;

struct node *find_node(pid_t pid){
    struct node *current;
   
    for(current = head; ((current->next != head) && (current->pid != pid)); current = current->next);
    
    return(current);
}

void change_process(pid_t pid){
    struct node *temp;
    
    if(head == NULL){
        return;
    }
    
    temp = find_node(pid);
    if(temp->next == temp){
        kill(pid, SIGCONT);
        return;
    }
    temp->running = 0; //indicates that the process has been stopped
    temp->next->running = 1; 
    kill(temp->pid, SIGSTOP);
    kill(temp->next->pid, SIGCONT);
}

void alarm_handler(int signum){
    struct node *curr;
    
    printf("caught alarm\n");
    
    if(head == NULL){
        return;
    }
    
    for(curr = head; curr->running != 1; curr = curr->next);
    
    change_process(curr->pid);
    signal(SIGALRM, alarm_handler);
    alarm(20);
}

void insert_node(pid_t pid, char **args, int num){
    struct node *new;
    
    new = (struct node *)malloc(sizeof(struct node));
    if(new == NULL){
        printf("Malloc problem in insert\n");
        exit(1);
    }
    
    new->pid = pid;
    new->orismata = args;
    new->args_num = num;
    
    if(head == NULL){
        new->next = new->prev = new;
        head = new;
        head->running = 1;
//         kill(head->pid, SIGCONT);
        signal(SIGALRM, alarm_handler);
        alarm(20);
    }else if(head->next == head){
        head->next = head->prev = new;
        new->prev = new->next = head;
        new->running = 0;
    }else{
        new->next = head;
        new->prev = head->prev;
        head->prev->next = new;
        head->prev = new;
        new->running = 0;
    }
    
    return;
}

void delete_node(pid_t pid){
    struct node *current;
    
    for(current = head; current->pid != pid; current = current->next){}
    
    if(current==head){
        if(head->next != head){
            head->prev->next = head->next;
            head->next->prev = head->prev;
            head = head->next;
        }else{head = NULL;}
        return;
    }

    current->next->prev = current->prev;
	current->prev->next = current->next;
    
    free(current);
}

void print_all(args_num){
    struct node *current;
    int i;
    
    if(head == NULL){
        printf("Empty list!\n");
        return;
    }
            
    printf("pid: %d, name: (./%s ", head->pid, head->orismata[0]);
    for(i=1; i<head->args_num; i++){
        printf(", %s", head->orismata[i]);
    }
    printf(")");
    //checking if head is the running process
    if(head->running == 1){
        printf(" (R)\n");
    }else{
        printf("\n");
    }
    if(head->next != head){
        for(current = head->next; current != head; current = current->next){
            printf("pid: %d, name: (./%s ", current->pid, current->orismata[0]);
            for(i=1; i<current->args_num; i++){
                printf(", %s", current->orismata[i]);
            }
            printf(")");
            //checking if current node is the running process
            if(current->running == 1){
                printf(" (R)\n");
            }else{
                printf("\n");
            }
        }
    }
}
void destroy(char **arguments, int num){
    struct node *current, *temp;
    int i;
    
    printf("exiting..\n");
    
    if(head == NULL){
        return;
    }
    
    if(head->next == head){
        if(kill(head->pid, SIGKILL) < 0){
            perror("kill SIGKILL 1");
        }
        return;
    }
    
    current = head;
    do{
        temp = current->next;
        if(kill(current->pid, SIGKILL) < 0){
            perror("kill SIGKILL 2");
        }
        free(current);
        current = temp;
    }while(current != head);
    
    for(i=0; i<num; i++){
        free(arguments[i]);
    }
    free(arguments);
    head = NULL;
}

static void sig_handler(int sig){
    struct node* current;
    
    signal(SIGUSR1, SIG_IGN);
    if(head == NULL){
        return;
    }
    if(kill(head->pid, SIGUSR1) < 0){
        perror("kill SIGUSR1 1");
    }
    
    for(current = head->next; current != head; current = current->next){
        if(kill(current->pid, SIGUSR1) < 0){
            perror("kill SIGUSR1 2");
        }
    }
    
    signal(SIGUSR1, sig_handler);
}

int main(int argc, char *argv[]){
    char choice[MAX_CHAR];
    char format[20];
    char progname[NAME_SIZE];
    int i, count = 2, strings_read = 1, pl;
    char **arguments;
    char ch = 'a', *delim;
    pid_t pid_pro, gtpd, given_pd;
    struct sigaction act = {{0}};
    struct pollfd fd[1];
    pid_t child_pid;
    int status;
    struct node *curr;
    
    fd[0].fd = STDIN_FILENO;
    fd[0].events = POLLIN;
    
    
    sprintf(format, "%%%ds", MAX_CHAR - 1);
    
    do{
        for(i=0; i<4; i++){
            choice[i] ='\0';
        }
        
        while ((child_pid = waitpid( -1, &status, WNOHANG | WUNTRACED)) > 0){  
        
            if (WIFEXITED(status)){

                curr = find_node(child_pid);
                if(curr->running == 1){
                    signal(SIGALRM, alarm_handler);
                    alarm(20); //restarting alarm in case of exit
                    change_process(child_pid);
                }
                delete_node(child_pid);
            }
            else if (WIFSTOPPED(status)){
                change_process(child_pid);
            }
            else if (WIFSIGNALED(status)){
                curr = find_node(child_pid);
                if(curr->running == 1){
                    signal(SIGALRM, alarm_handler);
                    alarm(20); //restarting alarm in case process has been terminated by signal
                    change_process(child_pid);
                }
                delete_node(child_pid);
            }
            else{
                perror("waitpid");
            }
        }
        //checking if there is input in the STDIN otherwise keeps on the checking the children using waitpid
        pl = poll(fd, 1, 2 * 1000);
        if(pl < 0 && errno != EINTR){
            perror("poll error");
        }
        
        if(pl == 0){
            continue;
        }
        
        if(fd[0].revents && POLLIN){
            
            scanf(format, choice);
            
            if(strcmp(choice, "exec") == 0){
                strings_read = 1;
                
                arguments = malloc(count * sizeof(char*));
                if(arguments == NULL){
                    fprintf(stderr, "Memory problem 1!\n");
                }
                for(i=0; i<count; i++){
                    arguments[i] = malloc(NAME_SIZE * sizeof(char));
                    if(arguments[i] == NULL){
                        fprintf(stderr, "Memory problem 2!\n");
                    }
                }
                scanf("%s", progname); 
                delim = strrchr(progname, '/');
                if(delim!=NULL){
                    strcpy(arguments[0], delim+1);
                }
                else{
                    strcpy(arguments[0],progname);
                }
                ch = fgetc(stdin);
                //reads till newline character is found which indicates end of input
                for(i=1; ch != '\n'; i++){
                    
                    if(ch == ' '){
                        //means a new argument is expected to be read
                        strings_read++;
                        if(strings_read > count){
                            //adjusting array size
                            arguments = realloc(arguments, ((strings_read + 1) * sizeof(char*)));
                            if(arguments == NULL){
                                fprintf(stderr, "Realloc problem!\n");
                            }
                            arguments[strings_read-1] = malloc(NAME_SIZE * sizeof(char));
                            if(arguments[strings_read-1] == NULL){
                                fprintf(stderr, "Memory problem 3!\n");
                            }
                            
                            scanf("%s", arguments[strings_read-1]);
                        }else{
                            scanf("%s", arguments[strings_read-1]);
                        }
                    }
                    arguments[strings_read] = NULL; //setting the value of the last argument in the array to NULL
                    ch = fgetc(stdin);
                }
                
                gtpd = getpid();
                printf("PARENT PID : %d\n", gtpd);
                
                pid_pro = fork();
                
                act.sa_handler = SIG_DFL; //children should inherit SIGUSR1 to its default state
                sigaction(SIGUSR1, &act, NULL);
            
                if(pid_pro == 0){
                    
                    if(execvp(progname, arguments) < 0){
                        perror("exec");
                        break;
                    }
                    
                }else if(pid_pro > 0){
                    //stops every new process until alarm goes off
                    if(kill(pid_pro, SIGSTOP) < 0){
                        perror("kill");
                    }
                    signal(SIGUSR1, sig_handler);
                    
                    insert_node(pid_pro, arguments, strings_read);
                }
            }else if(strcmp(choice, "list")==0){
                print_all();
            }else if(strcmp(choice, "term")==0){
                scanf("%d", &given_pd);
                kill(given_pd, SIGTERM);
            }else if(strcmp(choice, "sig")==0){
                scanf("%d", &given_pd);
                kill(given_pd, SIGUSR1);
            }
            else if(strcmp(choice, "quit") == 0){
                destroy(arguments, strings_read);
            }
        }
    }while(strcmp(choice, "quit") != 0);
 
    return 0;
}
