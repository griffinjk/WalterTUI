CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g

SRC = src/tui.c
OBJ = $(SRC:.c=.o)

TEST = tests/tuitest.c

OUT = test

all: $(OUT)

$(OUT): $(OBJ) $(TEST)
	$(CC) $(CFLAGS) $(OBJ) $(TEST) -o $(OUT)


clean:
	rm -f $(OBJ) $(OUT)
