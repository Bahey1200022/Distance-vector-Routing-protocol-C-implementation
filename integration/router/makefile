# -------------------------------
# Distance Vector Router Makefile
# -------------------------------

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -pthread

# Linker flags
LDFLAGS = -pthread

# Source files
SRC = main.c routing.c 
OBJ = $(SRC:.c=.o)

# Output executable
OUT = router

# Default target
all: $(OUT)

# Build the executable from object files
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)
	@echo "Distance Vector Router Built Successfully!"

# Compile .c -> .o with header dependencies
%.o: %.c neighbor_detection.h distance_table.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(OUT)
	@echo "Build cleaned!"
