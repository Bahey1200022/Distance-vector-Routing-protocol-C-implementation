CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

SRC = main.c dv.c
OBJ = $(SRC:.c=.o)
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c dv.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

run: $(EXEC)
	./$(EXEC)
