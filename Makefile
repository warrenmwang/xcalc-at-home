CC = gcc
CFLAGS = -lX11

SRC = main.c
TARGET = main.out

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) 

# OBJ = main.o

# $(TARGET): $(OBJ)
# 	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)
#
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
