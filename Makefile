# Makefile for wallserver.cpp with messages.h

# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -std=c++11

# Source files
SRCS = wallserver.cpp

# Header files
HDRS = messages.h

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = wallserver

# Default target
all: $(EXEC)

# Compile .cpp files to object files
%.o: %.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executable
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

# Clean up
clean:
	rm -f $(OBJS) $(EXEC)
