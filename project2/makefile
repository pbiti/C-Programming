OBJ = dirlist p2unarchive p2archive p2crypt hw2
all: $(OBJ)

dirlist.o: dirlist.c helper.h
	gcc -Wall -g -c dirlist.c
	
p2archive.o: p2archive.c helper.h
	gcc -Wall -g -c p2archive.c
	
p2unarchive.o: p2unarchive.c helper.h
	gcc -Wall -g -c p2unarchive.c
	
p2crypt.o: p2crypt.c helper.h
	gcc -Wall -g -c p2crypt.c
	
helper.o: helper.c helper.h
	gcc -Wall -g -c helper.c
	
dirlist: dirlist.o helper.o
	gcc -Wall -g dirlist.o helper.o -o dirlist
	
p2archive: p2archive.o helper.o
	gcc -Wall -g p2archive.o helper.o -o p2archive
	
p2unarchive: p2unarchive.o helper.o
	gcc -Wall -g p2unarchive.o helper.o -o p2unarchive
	
p2crypt: p2crypt.o helper.o
	gcc -Wall -g p2crypt.o helper.o -o p2crypt
	
hw2.o: hw2.c dirlist.o p2archive.o p2unarchive.o p2crypt.o helper.h
	gcc -Wall -g -c hw2.c
	
hw2: hw2.o helper.o
	gcc -Wall -g hw2.o helper.o -o hw2
