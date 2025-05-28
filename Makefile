CC = gcc
SRC_DIRS = com configuration types utility controller

CFLAGS = $(addprefix -I./, $(SRC_DIRS))
SRC = $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJ = $(SRC:.c=.o)


# Default target
all: debug

# Release build
release: CFLAGS += -O2 -Wall
release: myprogram

# Debug build
debug: CFLAGS += -g -O0 -Wall
debug: myprogram

myprogram: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(addsuffix /*.o, $(SRC_DIRS)) myprogram