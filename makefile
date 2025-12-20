# Compiler
CC := gcc

# -------------------------
# Source files
# -------------------------

# Engine core (shared)
ENGINE_SRC := \
	core/core_util.c \
	core/board.c \
	core/attack.c \
	core/move_gen.c \
	core/move_filter.c \
	core/move_apply.c 	

# GUI-specific
GUI_SRC := \
	main.c \
	GUI/chessboard.c \
	GUI/events.c \
	util/fps_counter.c \
	util/config_reader.c

# PERFT-specific
PERFT_SRC := \
	util/perft.c

# -------------------------
# Object files
# -------------------------
ENGINE_OBJ := $(ENGINE_SRC:.c=.o)
GUI_OBJ    := $(GUI_SRC:.c=.o)
PERFT_OBJ  := $(PERFT_SRC:.c=.o)

# -------------------------
# Targets
# -------------------------
GUI_TARGET   := ChessGUI
PERFT_TARGET := perft

# Includes / libs
INCLUDES := -I/usr/local/include/SDL3 -I/usr/local/include/SDL3_image
LIBS := -L/usr/local/lib -lSDL3_image -lSDL3

CFLAGS := -Wall -Wextra -O3 -fopenmp $(INCLUDES)
LDFLAGS := -fopenmp $(LIBS)

# -------------------------
# Rules
# -------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Default target
all: $(GUI_TARGET)

# GUI build
$(GUI_TARGET): $(ENGINE_OBJ) $(GUI_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# PERFT build (NO GUI, NO SDL)
$(PERFT_TARGET): $(ENGINE_OBJ) $(PERFT_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(ENGINE_OBJ) $(GUI_OBJ) $(PERFT_OBJ) $(GUI_TARGET) $(PERFT_TARGET)

run: $(GUI_TARGET)
	./run.sh

run-perft: $(PERFT_TARGET)
	./perft

