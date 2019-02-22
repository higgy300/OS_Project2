CFLAGS = -O
CC = g++
SRC = SampleTestP2.cpp MemoryManager.cpp
OBJ = $(SRC:.cpp = .o)

memory_manager: $(OBJ)
	$(CC) $(CFLAGS) -o memorymanager $(OBJ)

clean:
	rm -f core *.o
