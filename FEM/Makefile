CC = g++
LD = g++

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
TARGET = FEM

CFLAGS = -g -Wall -std=c++11
LDLIBS = -lGL -lglut
INCLUDES = -Iutils

SRC_DIR = 
BUILD_DIR = 
VPATH = utils

# Rules
all: $(TARGET)

$(TARGET).o: $(TARGET).cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

clean:
	rm -f *.o $(TARGET)

.PHONY: clean

# Dependencies
$(TARGET): $(OBJ) 

