# Files to compile
OBJS = src/main.c

# Compiler being used
CC = gcc

# Compilation flags being used
COMPILE_FLAGS = -w

# Linker flags for libs
LINKER_FLAGS = -lSDL2

# Name of executable
EXEC_NAME = chip8

# Target to compile and produce exec
# TODO: Cleanup via making debug build optional
all: $(OBJS)
	$(CC) $(OBJS) $(COMPILE_FLAGS) $(LINKER_FLAGS) -o $(EXEC_NAME) -g

clean:
	rm ./$(EXEC_NAME)
