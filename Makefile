CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -MMD -MP -pthread
LDLIBS = -lncurses

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
TARGET = build/idp

MODULOS = $(filter-out build/main.o,$(OBJ))
TEST_SRC = $(wildcard tests/*.c)
TEST_BIN = $(patsubst tests/%.c,build/%,$(TEST_SRC))
TSAN_BIN = build/test_concorrencia_tsan
TSAN_CFLAGS = $(CFLAGS) -O1 -fsanitize=thread
TSAN_LDFLAGS ?=

.PHONY: all run clean test test-tsan

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

build/%.o: src/%.c | build
	@$(CC) $(CFLAGS) -c $< -o $@

build/%: tests/%.c $(MODULOS) | build
	@$(CC) $(CFLAGS) -o $@ $< $(MODULOS) $(LDLIBS)

build:
	@mkdir -p build

run: all
	@./$(TARGET)

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "== $$t"; ./$$t || exit 1; done

$(TSAN_BIN): tests/test_concorrencia.c src/estacao.c \
		include/estacao.h include/idp.h | build
	@$(CC) $(TSAN_CFLAGS) $(TSAN_LDFLAGS) -o $@ \
		tests/test_concorrencia.c src/estacao.c

test-tsan: $(TSAN_BIN)
	@TSAN_OPTIONS="halt_on_error=1:exitcode=66" ./$(TSAN_BIN)

clean:
	@rm -rf build

-include $(DEP)
