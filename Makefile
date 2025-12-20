# Makefile for cSessionHop
# Author: Jacek Halon (jack_halon)

# Determine if this is a Windows or *NIX machine
ifeq ($(OS),Windows_NT)
	CC_x64 := x86_64-w64-mingw32-gcc.exe
	STR_x64 := x86_64-w64-mingw32-strip.exe
	CC_x86 := i686-w64-mingw32-gcc.exe
	STR_x86 := i686-w64-mingw32-strip.exe
	RM := del
else
	CC_x64 := x86_64-w64-mingw32-gcc
	STR_x64 := x86_64-w64-mingw32-strip
	CC_x86 := i686-w64-mingw32-gcc
	STR_x86 := i686-w64-mingw32-strip
	RM := rm
endif

build:
	@echo [+] Building cSessionHop...
	$(CC_x64) -o cSessionHop.x64.o -c src/cSessionHop.c -Os -s -w -Wno-multichar
	$(STR_x64) -N src/cSessionHop.c cSessionHop.x64.o -w
	$(CC_x86) -o cSessionHop.x86.o -c src/cSessionHop.c -Os -s -Wno-multichar
	$(STR_x64) -N src/cSessionHop.c cSessionHop.x86.o

clean:
	@echo [+] Cleaning Solution...
	$(RM) *.o

