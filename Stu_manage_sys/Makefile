# Compiler and flags
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2
TARGET   = stu_manage_sys
SRC      = Stu_manage_sys.c

# Use .exe on Windows
ifeq ($(OS),Windows_NT)
TARGET := $(TARGET).exe
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Run against the bundled sample data without touching it
run: $(TARGET)
	./$(TARGET) data/Grade.txt

clean:
	rm -f $(TARGET) stu_manage_sys.exe
