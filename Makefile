CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -MMD -MP -pthread

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
TARGET = build/idp

MODULOS = $(filter-out build/main.o,$(OBJ))
TEST_SRC = $(wildcard tests/*.c)
TEST_BIN = $(patsubst tests/%.c,build/%,$(TEST_SRC))

.PHONY: all run clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%: tests/%.c $(MODULOS) | build
	$(CC) $(CFLAGS) -o $@ $< $(MODULOS)

build:
	mkdir -p build

run: all
	./$(TARGET)

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "== $$t"; ./$$t || exit 1; done

clean:
	rm -rf build

-include $(DEP)
