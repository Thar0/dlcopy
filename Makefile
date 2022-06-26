# Makefile for test program

CC := gcc

TARGET := zobjcopy

SRC_DIRS := $(shell find src -type d)
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) TEST.c
O_FILES := $(foreach f,$(C_FILES:.c=.o),build/$f)

OPTFLAGS := -Wall -O3 -ffunction-sections -fdata-sections

$(shell mkdir -p build $(foreach dir,$(SRC_DIRS),build/$(dir)))

.PHONY: all clean
.DEFAULT_GOAL: all

all: $(TARGET)

clean:
	$(RM) -r build $(TARGET)

$(TARGET): $(O_FILES)
	$(CC) -Wl,--gc-sections $^ -o $@

build/%.o: %.c
	$(CC) $(OPTFLAGS) -I. -Isrc -c $< -o $@
