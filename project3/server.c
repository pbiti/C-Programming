/*Authors : Biti Polyxeni : 2582, Tasiou Evangelia : 2636
 *Date    : 5/5/2018
 * Program that handles multiple agents and receives information about flight bookings
 */
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<poll.h>
#include<sys/sem.h>


struct shared_mem{
    char airline[3];
    char dep[4];
    char arrive[4];
    int stops;
    int tickets;
};
struct agent_info{
	pid_t pid;
	int total_tickets;
};

int main(int argc, char *argv[]){
    int main_skt, ac_skt, list, shmid, shm_det, shm_ctrl, seek, i, count=0, pl = 5, tickets = 0, recv_pid, sem_id, sem_ctl, j, rec, k, send_info, cl;
    struct sockaddr_un addr;
    pid_t check_pid;
    key_t key, sem_key;
    struct shared_mem *p;
	struct agent_info ticket_array[atoi(argv[1])];
	FILE* fp;
    char ch, buf;
	nfds_t nfds = 2;
    struct pollfd fds[atoi(argv[1])+2];

	if(argc < 4){
		fprintf(stderr, "Invalid arguments!\n");
		exit(1);
	}
    unlink(argv[3]);

	for(i=0; i<atoi(argv[1]); i++){
		ticket_array[i].total_tickets = 0;
	}

    key = ftok(argv[0], 'c');
	if(key < 0){
		perror("ftok");
	}
	sem_key = ftok(argv[0], 'd');
	if(key < 0){
		perror("ftok");
	}
    sem_id = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if(sem_id < 0){
        perror("semget");
        exit(1);
    }
    sem_ctl = semctl(sem_id, 0, SETVAL, 1);
    if(sem_ctl < 0){
        perror("semctl");
        exit(1);
    }
	fp = fopen(argv[2], "r+");
	if(fp == NULL){
		perror("fopen");
		exit(1);
	}

	while(!feof(fp)){
		ch = fgetc(fp);
		if(ch=='\n'){
			count++;
		}
	}

	shmid = shmget(key, (size_t)(count*sizeof(struct shared_mem)), IPC_CREAT | IPC_EXCL | 0666);
	if(shmid < 0){
		perror("shmget");
	}

	p = (struct shared_mem *)shmat(shmid, NULL, 0);
	if(p == (struct shared_mem *)(-1)){
		perror("shmat");
	}

	seek = fseek(fp, 0, SEEK_SET);
	if(seek != 0){
            perror("fseek to start");
		exit(1);
        }
	for(i=0; i<count; i++){
		fscanf(fp,"%s %s %s %d %d", p[i].airline, p[i].dep, p[i].arrive, &(p[i].stops), &(p[i].tickets));
	}

    main_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    if(main_skt < 0){
        perror("Opening stream socket");
        exit(1);
    }

    if(argc < 3){
        write(STDERR_FILENO, "Invalid arguments!\n", 19);
        exit(1);
    }

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[3]);

    bind(main_skt, (struct sockaddr *)&addr, sizeof(addr)); //sizeof struct sockaddr_un + check for bind
    printf("Socket has name %s\n", addr.sun_path);

    list = listen(main_skt, atoi(argv[1]));
    if(list < 0){
        perror("listen");
        exit(3);
    }

	fds[1].fd = main_skt;
	fds[1].events = POLLIN | POLLHUP;
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;


	while(1){
		pl = poll(fds, atoi(argv[1])+2, 5 * 1000);
		if(pl < 0){
			perror("Poll");
			exit(1);
		}

		while(pl == 0){
			sleep(3);
			pl = poll(fds, nfds, 5 * 1000);
			if(pl < 0){
				perror("Poll");
				exit(1);
			}
		}

		for(i=0; i<atoi(argv[1]) + 2; i++){
			if(fds[i].revents & POLLIN){
				if(fds[i].fd == main_skt){
					ac_skt = accept(fds[i].fd, NULL, NULL);
					if(ac_skt < 0){
						perror("Accept");
						exit(1);
					}
					if(nfds + 1 > atoi(argv[1]) + 2){
						send_info = send(ac_skt, "Max agent limit has been reached!", 33 * sizeof(char), 0);
                        if(send_info < 0){
                            perror("Sending connection message to client");
                            exit(1);
                        }
						cl = close(ac_skt);
                        if(cl < 0){
                            perror("Closing accept socket");
                            exit(1);
                        }
					}else{
						send_info = send(ac_skt, "Connected to server.", 21 * sizeof(char), 0);
                        if(send_info < 0){
                            perror("Sending connection message to client");
                            exit(1);
                        }
                        /*topothethsh tou neou accept socket se thesh ston pinaka fds*/
						fds[nfds].fd = ac_skt;
						fds[nfds].events = POLLIN | POLLHUP;
						recv_pid = recv(fds[nfds].fd, &(ticket_array[nfds-2].pid), sizeof(pid_t), 0);
						if(recv_pid > 0){
							printf("Agent %d is connected.\n", ticket_array[nfds-2].pid);
						}else{
							perror("Recv pid");
							exit(1);
						}
						send_info = send(ac_skt, &key, sizeof(key_t), 0);
                        if(send_info < 0){
                            perror("Sending memory key to client");
                            exit(1);
                        }
						send_info = send(ac_skt, &sem_key, sizeof(key_t), 0);
                        if(send_info < 0){
                            perror("Sending semaphore key to client");
                            exit(1);
                        }
						send_info = send(ac_skt, &count, sizeof(int), 0);
                        if(send_info < 0){
                            perror("Sending count to client");
                            exit(1);
                        }
						nfds++;
					}
				}
				if(fds[i].fd == STDIN_FILENO){
                    read(STDIN_FILENO, &buf, sizeof(char));
                    if(buf == 'Q'){
                        printf("Exiting\n");
                        /*Tupwsh olwn twn telikwn eishthriwn twn energwn agent*/
						for(j=0; j<atoi(argv[1]); j++){
                            if(ticket_array[j].total_tickets != 0){
                                printf("Agent %d booked %d tickets.\n", ticket_array[j].pid, ticket_array[j].total_tickets);
                            }
						}
                        shm_ctrl = shmctl(shmid, IPC_RMID, NULL);
                        if(shm_ctrl < 0){
                            perror("shmctl");
                            exit(1);
                        }
                        shm_det = shmdt(p);
                        if(shm_det <  0){
                            perror("shmdt");
                            exit(1);
                        }
                        sem_ctl = semctl(sem_id, 0, IPC_RMID);
                        if(sem_ctl < 0){
                            perror("semctl");
                            exit(1);
                        }
                        fclose(fp);
                        close(main_skt);
                        if(cl <0){
                            perror("closing main socket");
                            exit(1);
                        }
                        return 0;
                    }
                }
                if(i > 1){
					rec = recv(fds[i].fd, &check_pid, sizeof(pid_t), 0);
                    if(rec < 0){
						perror("receiving pid");
						exit(1);
					}
					rec = recv(fds[i].fd, &tickets, sizeof(int), 0);
                    if(rec < 0){
                        perror("receiving booked tickets");
                        exit(1);
                    }
                   
                    if(tickets != 0){
                        /*apothikeush twn eishthriwn pou ekleise o agent sthn thesh pou tou antistoixei ston pinaka*/
                        for(j=0; j<atoi(argv[1]); j++){
                            if(ticket_array[j].pid == check_pid){
                                printf("Agent %d booked %d tickets.\n", ticket_array[j].pid, tickets);
                                ticket_array[j].total_tickets += tickets;
                            }
                        }

                        seek = fseek(fp, 0, SEEK_SET);
                        if(seek != 0){
                            perror("fseek to start");
                            exit(1);
                        }
                        /*Antigrafh twn newn dedomenwn ths koinoxrhsths mnhmhs sto arxeio*/
                        for(k=0; k<count; k++){
                            fprintf(fp, "%s %s %s %d %d\n", p[k].airline, p[k].dep, p[k].arrive, p[k].stops, p[k].tickets);
                        }
                    }else{
                        /*Euresh tou pid tou agent pou thelei na apoxwrhsei ston pinaka me tis plhrofories twn agents*/
                        
                        printf("Agent %d has been disconnected.\n", check_pid);
                        
                        for(j=0; j<=atoi(argv[1]); j++){
                            if(ticket_array[j].pid == check_pid){
                                ticket_array[j].total_tickets = 0;
                                break;
                            }
                        }
                        
                        /*Metakinhsh olwn twn epomenwn agents kata mia thesh aristera wste na eleutherwthei thesh gia neo agent*/
                        for(k=j; k<atoi(argv[1])-1; k++){
                            ticket_array[k] = ticket_array[k+1];
                        }
                        ticket_array[k+1].total_tickets = 0;
                        cl = close(fds[i].fd);
                        if(cl < 0){
                            perror("Closing accept socket from agent that exited");
                            exit(1);
                        }
                        
                        nfds--;
                        
                        /*Metakinhsh twn accept sockets pou exoume apothikeusei ston pinaka fds wste na vriskontai se antistoixia me ta stoixeia tou pinaka ticket_array*/
                        for(k=j+2; k<=atoi(argv[1])+1; k++){
                            fds[k] = fds[k+1];
                        }
                       
                    }
                }
        
            }else{
				continue;
			}
        }
    }
}
