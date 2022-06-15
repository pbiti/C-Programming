/*Program that handles a database 
 * Authors: Tasiou Evangelia_02636, Biti Polyxeni_02582
 * Date: 11 Mar 2018
 */
#include<stdio.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#define MAX_LENGTH 250
int find_file(char name[], int fd1, int export, int del);
void print_menu(){
	printf("i(mport)<name>:\nf(ind)<name>:\ne(xport)<src><dest>:\n");
	printf("d(elete)<name>:\nq(uit):\n");		
}

/* import_file(path[], fd1) 
 * 
 * Purpose: imports new object to the database
 * 
 * Parameters: path[]- absolute or relative path of the file to be imported
 *             fd1   - file descriptor of the database
 */
void import_file(char path[], int fd1){
	int fd_file, sync, count;
	ssize_t wr1, rd=3, rd1=-1;
	int i=1, cl1, name_length;
	char *data, *delim, letter_read = '\0';
    off_t move_to_start, ls_end, file_size, ls2;
	
	delim = strrchr(path, '/');
	
	move_to_start = lseek(fd1, 0, SEEK_SET);
	if(move_to_start<0){
		perror("lseek to start of the database");
		exit(1);
	}
	
	if(delim!=NULL){
		name_length = strlen(delim) - 1; //strrchr returns the position of the last '\' char
		while(rd!=0 && i<name_length){   //searching if the file name already exists in the database
            rd = read(fd1, &letter_read, 1);
			if(rd<0){
				perror("reading character");
				exit(1);
			}
			if(letter_read==delim[i]){
				i++;
			}else{ i = 1; }
		}
	}else{
		name_length = strlen(path);
		while(rd!=0 && i<name_length){
            rd = read(fd1, &letter_read, 1);
			if(rd<0){
				perror("reading character");
				exit(1);
			}
			if(letter_read==path[i]){
				i++;
			}else{ i = 1; }
		}
	}
	if(i==name_length){
		printf("file already exists in database\n");
		exit(1);
	}

	fd_file = open(path, O_RDONLY);
	if(fd_file<0){
		perror("opening the file");
		exit(1);
	}
	
	ls_end = lseek(fd1, 0, SEEK_END); //import the new file at the end of the database
	if(ls_end<0){
		perror("lseek to end of the database");
		exit(1);
	}
	
	wr1 = write(fd1, "magic", 5);
	if(wr1<0){
		perror("write magic number");
		exit(1);
	}
	
	wr1 = write(fd1, &name_length, sizeof(int)); //writing the name size of the file imported
	if(wr1<0){
		perror("write name length");
		exit(1);
	}
	
	if(delim==NULL){
        wr1 = write(fd1, path, strlen(path));
	}
	else {
		wr1 = write(fd1, delim+1, name_length);
	}
	
	if(wr1<0){
		perror("write file name");
		exit(1);
	}
	file_size = lseek(fd_file, 0, SEEK_END); //finding the size of the file to be imported
	if(file_size<0){
		perror("lseek to find size of the file");
		exit(1);
	}
	
	wr1 = write(fd1, &file_size, sizeof(int));
	if(wr1<0){
		perror("write size of file");
		exit(1);
	}
	
	sync = fsync(fd1);
	if(sync<0){
		perror("fsync");
		exit(1);
	}
	
	ls2 = lseek(fd_file, 0, SEEK_SET); //lseek to start to copy file's data to database
	if(ls2<0){
		perror("lseek to start of the file");
		exit(1);
	}
	
	data = (char *)malloc(512*sizeof(char));
	if(data==NULL){
		printf("Malloc problem!\n");
		exit(1);
	}
	
	for(count=0; count <= file_size && rd1!=0; count = count + rd1){
		rd1 = read(fd_file, data, 512);
		if(rd1<0){
			perror("read data from file");
			free(data);
			exit(1);
		}
		wr1 = write(fd1, data, rd1);
		if(wr1<0){
			perror("write data to database");
			free(data);
			exit(1);
		}
	}
	
	free(data);
	
	cl1 = close(fd_file);
	if(cl1<0){
		perror("closing the file that was imported");
        exit(1);
	}
}
/* find_file(name[], fd1, export, del)
 * 
 * Purpose: prints all names that contain a given string
 * 
 * Parameters: name[]- the string the user is looking for
 *             fd1   - the database's file descriptor
 *             export- flag that specifies if we're using the function to export a file
 *             del   - flag that specifies if we're using the function to delete a file
 */
int find_file(char name[], int fd1, int export, int del){
	ssize_t rd1 = 5;
	char find_name[MAX_LENGTH] = {'\0'};
	int name_size, file_size;
	int i;
    off_t next_file, ls;
	
	ls = lseek(fd1, 5, SEEK_SET); //start searching after the magic number
	if(ls<0){
		perror("lseek");
		exit(1);
	}
	
	while(rd1 != 0){
		for(i=0; i<MAX_LENGTH; i++){
			find_name[i] = '\0';
		}
		rd1 = read(fd1, &name_size, sizeof(int));
		if(rd1<0){
			perror("read name size");
			exit(1);
		}
		rd1 = read(fd1, find_name, name_size);
		if(rd1<0){
			perror("reading name");
			exit(1);
		}
		if(find_name[0] != '\0'){
			if(name[0] == '*'){
				printf("%s\n",  find_name);
			}else{
				if(strstr(find_name, name)!=NULL && export == 0 && del == 0){
					printf("%s\n", find_name);
				}else if(strcmp(find_name, name)==0 && export == 1){
					return(0);
				}else if(strcmp(find_name,name)==0 && del==1){
                    return(name_size);
                }
			}
		}
		rd1 = read(fd1, &file_size, sizeof(int));
		if(rd1 < 0){
			perror("reading size of the file");
			exit(1);
		}
		
		next_file = lseek(fd1, file_size + 5, SEEK_CUR); //compare to the next file's name
		if(next_file < 0){
			perror("lseek to the beggining of the next file");
			exit(1);
		}
	}
	return(1);
}
/* export_file(fd1, source[], dest[])
 * 
 * Purpose: exports an object from the database to another file that the user specifies
 * 
 * Parameters: fd1     - the database's file descriptor
 *             source[]- the name of the object to be exported
 *             dest[]  - the name of the file that the object is exported to
 */
void export_file(int fd1, char source[], char dest[]){
	
	int fd_dest, file_size, count = 0;
	ssize_t rd1, wr1;
	char *data;
	
	fd_dest = open(dest, O_RDWR | O_CREAT, 0600);
	if(fd_dest < 0){
		perror("open dest");
		exit(1);
	}
	
	rd1 = read(fd1, &file_size, sizeof(int));
	if( rd1 < 0 ){
		perror("reading file size");
		exit(1);
	}
	data = (char*)malloc(512*sizeof(char));
	if(data == NULL){
		printf("Memory problem!\n");
		exit(1);
	}

	for(count = 0; count < file_size; count = count + rd1){
		if(file_size <= 512){
            rd1 = read(fd1, data, file_size);
        }else{
            rd1 = read(fd1, data, 512);
        }
        if(rd1 < 0){
            perror("reading file to export its data");
            exit(1);
        }
		wr1 =  write(fd_dest, data, rd1);
		if(wr1 < 0){
			perror("writing data to dest file");
			exit(1);
		}
	}
	free(data);

}
/* delete_file( fd1, name[], name_size, db_size)
 * 
 * Purpose: deletes an object from the database
 * 
 * Parameters: fd1       - the database's file descriptor
 *             name[]    - the name of the object to be deleted
 *             name_size - the length of name[]
 *             db_size   - database's size
 */
void delete_file(int fd1, char mame[], int name_size, off_t db_size){

	int ftr, file_size, count=0, i;
	ssize_t rd1=5, wr;
    off_t next_file, tr_length, rd_pos, wr_stops, wr_pos, rd_again;
    char *data;
    
	rd1 = read(fd1, &file_size, sizeof(int)); 
	if(rd1 < 0 ){
		perror("reading size of the file to be deleted");
		exit(1);
	}
    
    next_file = lseek(fd1, (off_t)file_size, SEEK_CUR); 
    if(next_file<0){
        perror("lseek to next file's start");
        exit(1);
    }
    
    data = (char *)malloc(512);
    if(data==NULL){
        printf("Malloc problem!\n");
        exit(1);
    }
    
    while(rd1!=0){
        for(i=0; i<512; i++){
            data[i]='\0';
        }
        
        rd1 = read(fd1, data, 512);
        if(rd1<0){
            perror("reading data");
            exit(1);
        }
    
        rd_pos = lseek(fd1, 0, SEEK_CUR); //where reading stops
        if(rd_pos<0){
            perror("lseek to find where reading stopped");
            exit(1);
        }
        
        if(count>0){
            wr_pos = lseek(fd1, wr_stops, SEEK_SET); 
            if(wr_pos<0){
                perror("lseek to writing position");
                exit(1);
            }
        }
        if(count==0){
            wr_pos = lseek(fd1, -(rd1 + 5 + (2*sizeof(int)) + file_size + name_size), SEEK_CUR); 
            if(wr_pos<0){
                perror("lseek to writing position");
                exit(1);
            }
        }
        wr = write(fd1, data, rd1);
        if(wr<0){
            perror("replacing data");
            exit(1);
        }
        wr_stops = lseek(fd1, 0, SEEK_CUR);
        if(wr_stops<0){
            perror("lseek where writing stops");
            exit(1);
        }
        
        rd_again = lseek(fd1, rd_pos, SEEK_SET);
        if(rd_again<0){
            perror("lseek to pos to continue reading");
            exit(1);
        }
        
        count++;
    }
    tr_length = db_size - file_size - 5 - (2*sizeof(int)) - name_size;
    ftr = ftruncate(fd1, tr_length);
    if(ftr < 0){
        perror("ftruncate problem");
        exit(1);
    }
    free(data);
}

int main (int argc, char *argv[]){
	char choice='a';
	int fd1, cl1, file_exists, name_size;
	char path[MAX_LENGTH], dest[MAX_LENGTH];
	char format[25];
    off_t db_size;
	
	if(argc==1){
		printf("Invalid database name\n");
		exit(1);
	}

	fd1 = open(argv[1], O_RDWR | O_CREAT, S_IRWXU);
	if(fd1<0){
		perror("opening database");
		exit(1);
	}
	
	while(choice!='q'){
		print_menu();
		
		scanf(" %c", &choice);
		if(choice=='q'){
			cl1 = close(fd1);
			if(cl1<0){
				perror("closing database");
			}
			return 0;
		}
		while(choice!='i' && choice!='f' && choice!='e' && choice!='d'){
			printf("Invalid character!\n");
			scanf(" %c", &choice);
			if(choice=='q'){
				cl1 = close(fd1);
				if(cl1<0){
					perror("closing database");
				}
				return 0;
			}
		}
		
		sprintf(format, "%%%ds", MAX_LENGTH-1);
		scanf(format, path);
		if(choice=='e'){
			scanf(format,dest);
		}
		
		switch (choice){
			case 'i': {import_file(path, fd1); break;}
			case 'f': {find_file(path, fd1, 0, 0); break;}
			case 'e': {
				file_exists = find_file(path, fd1, 1, 0); 
				while(file_exists != 0){
					printf("File does not exist.\n");
                    scanf(format, path);
                    file_exists = find_file(path, fd1, 1, 0);
				} 
				export_file(fd1, path, dest);
				break;
			}
			case 'd': {
                db_size = lseek(fd1, 0, SEEK_END);
                if(db_size< 0){
                    perror("lseek to end");
                    exit(1);
                }
				file_exists = find_file(path, fd1, 1, 0);
				while(file_exists != 0){
					printf("File does not exist.\n");
                    scanf(format, path);
                    file_exists = find_file(path, fd1, 1, 0);
				}
				name_size = find_file(path, fd1, 0, 1);
				delete_file(fd1, path, name_size, db_size);
				break;
			}
		}
	}
	return 0;
}

