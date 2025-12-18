# Compiler
CC := gcc

# Source files
SRC := main.c util/config_reader.c util/fps_counter.c \
			  GUI/chessboard.c GUI/events.c \
			  core/core_util.c core/board.c core/attack.c core/move_gen.c core/move_filter.c core/move_apply.c

OBJ := $(SRC:.c=.o)

# Target
TARGET := ChessEngine

# Adjust paths as needed
INCLUDES := -I/usr/local/include/SDL3 -I/usr/local/include/SDL3_image
LIBS := -L/usr/local/lib -lSDL3_image -lSDL3

# Compiler flags
CFLAGS := -Wall -Wextra -g -O0 $(INCLUDES)
LDFLAGS := $(LIBS)

# Rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./run.sh
