CC=i686-w64-mingw32-gcc
CC_64=x86_64-w64-mingw32-gcc

all: libcpltest.x86.zip libcpltest.x64.zip

bin:
	mkdir bin

#
# x86 targets
#
libcpltest.x86.zip: bin
	$(CC) -DWIN_X86 -shared -masm=intel -Wall -Wno-pointer-arith -c src/cpltest.c     -o bin/cpltest.x86.o
	zip -q -j libcpltest.x86.zip bin/*.x86.o

#
# x64 targets
#
libcpltest.x64.zip: bin
	$(CC_64) -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -c src/cpltest.c       -o bin/cpltest.x64.o
	zip -q -j libcpltest.x64.zip bin/*.x64.o

#
# Other targets
#
clean:
	rm -rf bin/*.o
	rm -f libcpltest.x86.zip
	rm -f libcpltest.x64.zip
