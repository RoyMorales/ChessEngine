# Compiler
CC := gcc

# -------------------------
# Source files
# -------------------------

# Board core (shared by all targets)
BOARD_SRC := \
	core/core_util.c \
	core/board.c \
	core/board_history.c \
	core/attack.c \
	core/move_gen.c \
	core/move_filter.c \
	core/move_apply.c \
	core/zobrist.c

# GUI (SDL3)
GUI_SRC := \
	main.c \
	GUI/chessboard.c \
	GUI/events.c \
	GUI/menu.c \
	util/fps_counter.c \
	util/config_reader.c \
	engine/eval.c

# UCI engine (no SDL — this is the engine binary)
UCI_SRC := \
	UCI/uci.c \
	UCI/main_uci.c \
	engine/eval.c

# Perft
PERFT_SRC := \
	util/perft.c

# WAC test suite
WAC_SRC := \
	util/wac.c

MEM_SRC := \
	mem_test.c

# -------------------------
# Object files
# -------------------------
BOARD_OBJ := $(BOARD_SRC:.c=.o)
GUI_OBJ   := $(GUI_SRC:.c=.o)
UCI_OBJ   := $(UCI_SRC:.c=.o)
PERFT_OBJ := $(PERFT_SRC:.c=.o)
WAC_OBJ   := $(WAC_SRC:.c=.o)

# -------------------------
# Binary names
# -------------------------
ENGINE  := CodFish       # UCI engine binary
GUI     := ChessGUI      # SDL3 GUI binary
PERFT   := perft
WAC     := wac
MEMTEST := mem_test

# -------------------------
# Compiler flags
# -------------------------
INCLUDES      := -I/usr/local/include/SDL3 -I/usr/local/include/SDL3_image -I/usr/local/include/SDL3_ttf
SDL_LIBS      := -L/usr/local/lib -lSDL3 -lSDL3_image -lSDL3_ttf
CFLAGS        := -Wall -Wextra -O3 -fopenmp $(INCLUDES)
LDFLAGS_SDL   := -fopenmp $(SDL_LIBS)
LDFLAGS_PLAIN := -fopenmp -lm

# -------------------------
# Rules
# -------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Default: build everything
all: $(ENGINE) $(GUI) $(PERFT) $(WAC)

# UCI engine — the real engine binary
engine: $(ENGINE)
$(ENGINE): $(BOARD_OBJ) $(UCI_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS_PLAIN)

# SDL3 GUI
gui: $(GUI)
$(GUI): $(BOARD_OBJ) $(GUI_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS_SDL)

# Perft
perft: $(PERFT)
$(PERFT): $(BOARD_OBJ) $(PERFT_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS_PLAIN)

# WAC test suite
wac: $(WAC)
$(WAC): $(BOARD_OBJ) $(WAC_OBJ) engine/eval.o
	$(CC) $^ -o $@ $(LDFLAGS_PLAIN)

memtest: $(MEMTEST)
$(MEMTEST): $(BOARD_OBJ) mem_test.o
	$(CC) $^ -o $@ $(LDFLAGS_PLAIN)

# -------------------------
# Utility
# -------------------------
clean:
	rm -f $(BOARD_OBJ) $(GUI_OBJ) $(UCI_OBJ) $(PERFT_OBJ) $(WAC_OBJ) engine/eval.o \
	      $(ENGINE) $(GUI) $(PERFT) $(WAC)

run: $(GUI)
	./$(GUI)

run-engine: $(ENGINE)
	./$(ENGINE)

run-perft: $(PERFT)
	./$(PERFT)

run-wac: $(WAC)
	./$(WAC)

run-memtest: $(MEMTEST)
	./$(MEMTEST)

.PHONY: all engine gui perft wac clean run run-engine run-perft