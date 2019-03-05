CFLAGS = -O
CC = g++
SRC = SampleTestP2.cpp MemoryManager.cpp
OBJ = $(SRC:.cpp = .o)

memory_manager: $(OBJ)
	$(CC) $(CFLAGS) -o memorymanager -g -O0 $(OBJ)

clean:
	rm -f core *.o
