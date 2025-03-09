CC = gcc
CFLAGS = -Wall -lX11 -std=c99

SRC = main.c
TARGET = main.out

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm -f $(TARGET)
