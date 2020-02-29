# Makefile for compiling SDL2 apps

CC := x86_64-w64-mingw32-g++

# compiler flags
CFLAGS :=  `sdl2-config --cflags` -ggdb3 -O0 --std=c++17 -Wall -lm -l ./include/* --static

# headers
HDRS := ../*.h

# source files
SRCS := ../*.cpp

# generate names of object files
OBJS := $(SRCS:.c=.o)

# output executable name
EXEC := ../chip8emu.exe

# default recipe
all: $(EXEC)
 
showfont: showfont.c Makefile
	$(CC) -o $@ $@.c $(CFLAGS) $(LIBS)

glfont: glfont.c Makefile
	$(CC) -o $@ $@.c $(CFLAGS) $(LIBS)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#	$(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
