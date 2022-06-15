/* Authors: Biti Polyxeni : 2582, Tasiou Evangelia : 2636
 * Date: 05/05/2018
 * Program that represents an agent who finds and reserves tickets for available flights */

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/un.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<sys/sem.h>


struct shared_mem{
    char airline[3];
    char dep[4];
    char arrive[4];
    int stops;
    int tickets;
};

int main(int argc, char *argv[]){
    int main_skt, con, i, shmid, count, flight_exist = 0, sem_id, sem_op, send_num, cl, discon = 0;
    struct sockaddr_un addr;
    key_t num, sem_key;
    struct shared_mem *info, *data;
    struct sembuf op;
    pid_t pid;
    ssize_t rec, rec_semkey;
    char choice[8], msg[35];
    
	if(argc < 2){
		fprintf(stderr, "Invalid arguments!\n");
		exit(1);
	}
	
    for(i=0; i<8; i++){
        choice[i] = '\0';
    }
    for(i=0; i<35; i++){
		msg[i] = '\0';
	}
	main_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    if(main_skt < 0){
		perror("Opening stream socket");
		exit(1);
	}
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, argv[1]);
	
	con = connect(main_skt, (struct sockaddr *)&addr, sizeof(addr));
	if(con < 0){
		close(main_skt);
		perror("Connecting stream socket");
		exit(1);
	}
	rec = recv(main_skt, msg, sizeof(msg), 0);
	if(rec < 0){
		perror("Recieving connection msg");
		exit(1);
	}
	printf("%s\n", msg);
	pid = getpid();
	send_num = send(main_skt, &pid, sizeof(pid_t), 0);
    if(send_num < 0){
        perror("sending pid");
        exit(1);
    }
    
    rec = recv(main_skt, &num, sizeof(key_t), 0);
    if(rec < 0){
        perror("recv mem key");
        exit(1);
    }
    shmid = shmget(num, 0, 0);
    if(shmid < 0){
        perror("shmget");
        exit(1);
    }
    data = (struct shared_mem *)shmat(shmid, NULL, 0);
    if(data == (struct shared_mem *)(-1)){
        perror("shmat");
        exit(1);
    }
    
    rec_semkey = recv(main_skt, &sem_key, sizeof(key_t), 0);
    if(rec_semkey < 0){
        perror("recv sem_key");
        exit(1);
    }
    sem_id =  semget(sem_key, 0, 0);
    if(sem_id < 0){
        perror("semget");
        exit(1);
    }
    rec = recv(main_skt, &count, sizeof(int), 0);
    if(rec < 0){
        perror("recv");
    }
	
    do{
        
        flight_exist = 0;
        printf("(FIND) SRC DEST NUM\n(RESERVE) SRC DEST AIRLINE NUM\n(EXIT)\n");
        scanf("%s", choice);
            
        if(strcmp(choice, "FIND")==0){
			op.sem_num = 0;
			op.sem_op = -1;
			op.sem_flg = 0;
				
			sem_op = semop(sem_id, &op, 1);
			if(sem_op < 0){
				perror("semop");
				exit(1);
			}
            info = (struct shared_mem *)malloc(sizeof(struct shared_mem));
            scanf("%s %s %d", info->dep, info->arrive, &(info->tickets));
            for(i=0; i<count; i++){
                if((strcmp(info->dep, data[i].dep) == 0) && (strcmp(info->arrive, data[i].arrive) == 0) && (info->tickets <= data[i].tickets)){
                    fprintf(stdout,"%s %s %s %d %d\n", data[i].airline, data[i].dep, data[i].arrive, data[i].stops, data[i].tickets);
                    flight_exist++; //increasing flag's value to ckeck if there are available flights
                }
            }
            if(flight_exist == 0){
                fprintf(stderr, "No flights found!\n");
            }
            
			op.sem_num = 0;
			op.sem_op = 1;
			op.sem_flg = 0;
					
			sem_op = semop(sem_id, &op, 1);
			if(sem_op < 0){
				perror("semop");
				exit(1);
			}
			free(info);
        }
        else if(strcmp(choice, "RESERVE")==0){
			op.sem_num = 0;
			op.sem_op = -1;
			op.sem_flg = 0;
				
			sem_op = semop(sem_id, &op, 1);
			if(sem_op < 0){
				perror("semop");
				exit(1);
			}
            info = (struct shared_mem *)malloc(sizeof(struct shared_mem));
            scanf("%s %s %s %d", info->dep, info->arrive, info->airline, &(info->tickets));
            for(i=0; i<count; i++){
                if((strcmp(info->dep, data[i].dep) == 0) && (strcmp(info->arrive, data[i].arrive) == 0) && (strcmp(info->airline, data[i].airline) == 0) && (info->tickets <= data[i].tickets)){
                    printf("Agent reserved %d tickets\n", info->tickets);
                    data[i].tickets = data[i].tickets - info->tickets;
                    printf("Tickets left : %d\n", data[i].tickets);
					send_num = send(main_skt, &pid, sizeof(pid_t), 0);
                    if(send_num < 0){
                        perror("sending pid for reservation");
                        exit(1);
                    }
                    
                    send(main_skt, &(info->tickets), sizeof(int), 0); 
                    if(send_num < 0){
                        perror("sending pid for reservation");
                        exit(1);
                    }
                    flight_exist++; //increasing flag's value in case reservation was successful
					break;
                }
            }
            if(flight_exist == 0){
                fprintf(stderr, "Reservation not successful.\n");
            }else{
                printf("Reservation was successful!\n");
            }
            op.sem_num = 0;
			op.sem_op = 1;
			op.sem_flg = 0;
					
			sem_op = semop(sem_id, &op, 1);
			if(sem_op < 0){
				perror("semop");
				exit(1);
			}
            free(info);
        }
        else if(strcmp(choice, "EXIT")==0){
           
            send_num = send(main_skt, &pid, sizeof(pid_t), 0);
            if(send_num < 0){
                perror("sending pid for reservation");
                exit(1);
            }
            //sending 0 to server to indicate agent's exit
            send_num = send(main_skt, &discon, sizeof(int), 0);
            if(send_num < 0){
                perror("sending discon value");
                exit(1);
            }
            
            cl = close(main_skt);
            if(cl < 0){
                perror("closing socket end to exit");
            }
            
            return 0;
        } 
        else{
            fprintf(stderr, "Invalid Choice!\n");
        }
	}while(strcmp(choice, "EXIT") != 0);
    
    return 0;
}
