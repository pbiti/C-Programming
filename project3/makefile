OBJ = agent server
all: $(OBJ)

agent.o: agent.c 
	gcc -Wall -g -c agent.c
	
server.o: server.c 
	gcc -Wall -g -c server.c
	
agent: agent.o 
	gcc -Wall -g agent.o -o agent
	
server: server.o
	gcc -Wall -g server.o -o server
	

