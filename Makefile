TARGET := murmur3_test.exe

CC := clang
CSTD := c99
OPT := -O3
DEBUG := -g
WARNINGS := -Wall -Wextra -Werror -Wpedantic
WARNINGS += -Wno-cast-align -Wno-unused-macros

CFLAGS := -std=$(CSTD) $(OPT) $(DEBUG) -I./src $(WARNINGS)

all:
	$(CC) $(CFLAGS) ./src/murmur3.c ./src/murmur3_test.c -o ./bin/$(TARGET)

.PHONY: clean
clean:
	$(RM) $(OBJSCLEAN)
	$(RM) ./bin/*
