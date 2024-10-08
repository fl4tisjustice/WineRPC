SRC_DIR := src
BIN_DIR := bin

SRC := $(wildcard $(SRC_DIR)/**/**/*.c)
SRC += $(wildcard $(SRC_DIR)/**/*.c)
EXE := $(BIN_DIR)/winerpcbridge.exe

GIT_VERSION := "$(shell git describe --always --tags | sed -E 's/-[0-9]+-/-/')"

CC          :=      x86_64-w64-mingw32-gcc
CFLAGS      :=      -masm=intel -std=c99 -O3 -g -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wunreachable-code -pedantic -pedantic-errors
CPPFLAGS    :=      -Iinclude -DVERSION=\"$(GIT_VERSION)\"
LDFLAGS     :=      
    
.PHONY: all clean

all: $(EXE)
 
$(EXE): $(SRC) | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BIN_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BIN_DIR) *.s