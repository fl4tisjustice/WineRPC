SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/winerpcbridge
SRC := $(wildcard $(SRC_DIR)/**/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

CC			:=		x86_64-w64-mingw32-gcc
CFLAGS		:=		-masm=intel -std=c99 -O3 -g -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wunreachable-code -Wno-unused-parameter -pedantic -pedantic-errors
CPPFLAGS	:=		-Iinclude
LDFLAGS		:=		
	
.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR) *.s