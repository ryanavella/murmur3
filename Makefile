TARGET := murmur3_test.exe

CC := clang
CSTD := c99
OPT := -O3
DEBUG := -g
WARNINGS := -Wall -Wextra -Werror -Wpedantic
WARNINGS += -Wno-cast-align

DIR_SRC := ./src
DIR_INCL := ./include
DIR_BIN := ./bin
INCLUDE := -I$(DIR_SRC) -I$(DIR_INCL)

SRCS      := $(shell find $(DIR_SRC) -name "*.c")
OBJSCLEAN := $(shell find $(DIR_SRC) -name "*.o")
DEPSCLEAN := $(shell find $(DIR_SRC) -name "*.d")

CFLAGS := -std=$(CSTD) $(OPT) $(DEBUG) $(INCLUDE) $(WARNINGS) -MMD -MP

.PHONY: all
all: $(DIR_BIN)/$(TARGET)

$(DIR_BIN)/$(TARGET): $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@

-include $(SRCS:%.c=%.d)

.PHONY: format
format:
	clang-format -i *.c *.h

.PHONY: clean
clean:
	$(RM) $(OBJSCLEAN)
	$(RM) $(DIR_BIN)/*.exe
	$(RM) $(DIR_BIN)/$(TARGET)

.PHONY: veryclean
veryclean: clean
	$(RM) $(DEPSCLEAN)
