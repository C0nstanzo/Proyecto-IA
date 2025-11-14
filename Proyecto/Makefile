CC = g++
CFLAGS = -O2 -std=c++17 -Wall -Wextra
TARGET = solver
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) salida.txt

.PHONY: all clean
