CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/main.c src/mapa.c src/robo.c
TARGET = idp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
