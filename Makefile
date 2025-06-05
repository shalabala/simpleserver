CC = gcc

SRC_DIRS = com configuration types utility controller template dal
OBJ_DIRS = $(addprefix ./build/, $(SRC_DIRS))
OBJ_DIRS += build/src

CFLAGS = $(addprefix -I./, $(SRC_DIRS))
SRC = $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJ = $(SRC:.c=.o)

# Default target
all: debug

# Release build
release: CFLAGS += -O2 -Wall
release: simpleserver

# Debug build
debug: CFLAGS += -g -O0 -Wall
debug: simpleserver

test: CFLAGS += -g -O0 -Wall

ifeq ($(sanitize),true)
CFLAGS += -fsanitize=address -fsanitize=undefined
endif

dirs:
	@mkdir -p $(OBJ_DIRS)

test: dirs $(OBJ) $(mkdir $(OBJ_DIRS))
	$(CC) $(CFLAGS) $(addprefix ./build/, $(OBJ)) src/test.c -o test

simpleserver: dirs $(OBJ)
	$(CC) $(CFLAGS) $(addprefix ./build/, $(OBJ)) src/main.c -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o ./build/$@

clean:
	rm -f $(addsuffix /*.o, $(OBJ_DIRS)) test simpleserver