OBJS 	= main.o semaphores.o shared_memory.o
SOURCE	= main.c semaphores.c shared_memory.c
HEADER  = semaphores.h shared_memory.h
OUT  	= main
CC	= gcc
FLAGS   = -std=gnu99 -c -Wall 
# -c flag generates object code for separate files

$(OUT): $(OBJS)
	$(CC) $(OBJS) -o $@ -lcrypto

#Compile the individual files separately.
main.o: main.c
	$(CC) $(FLAGS) main.c 

semaphores.o: semaphores.c semaphores.h
	$(CC) $(FLAGS) semaphores.c
	
shared_memory.o: shared_memory.c shared_memory.h
	$(CC) $(FLAGS) shared_memory.c


clean:
	rm -f $(OBJS) $(OUT)
