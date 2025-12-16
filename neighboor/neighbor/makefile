# -------------------------------
# Neighbor Detection Makefile
# -------------------------------

CC = gcc
CFLAGS = -Wall -g -pthread
LDFLAGS = -pthread

# Sources
SRC = neighbor_detection.c main.c
OBJ = $(SRC:.c=.o)

# Output executable
OUT = neighbor_app

# Default target
all: $(OUT)

# Build executable
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)
	@echo "Neighbor Detection App Built"

# Compile .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(OUT)
	@echo "Cleaned Neighbor Detection build"
