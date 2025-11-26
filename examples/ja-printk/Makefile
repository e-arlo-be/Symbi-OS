CC = gcc

# lets go through these and make sure we understand what we really need -- if dpld logic
# requires mitigations then let's stack switch extra as needed
CFLAGS = -O0 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer -Wfatal-errors \
	-fPIC
CFLAGS += -I ../../Symlib/include -DDYNLINK

TARGETS :=  main main_static
all: $(TARGETS)

# extension boiler plate
libextension.a extension.h ext.kbin libkallsyms.so libkallsyms.a: $(wildcard *.kc *.kh)
	../tools/kcc

# main program
main.o: main.c extension.h 
	$(CC) $(CFLAGS) -c -o main.o main.c

main:  main.o libextension.a libkallsyms.so
	$(CC) $(CFLAGS) -o $@ $< -lc -L../../Symlib/dynam_build -lSym -L. -lkallsyms -lextension 

main_static: extension.h main.o libkallsyms.a
	$(CC) $(CFLAGS) -o $@ $^  libkallsyms.a -lc -L../../Symlib/dynam_build -lSym -L. -lextension 

clean:
	rm -rf $(wildcard $(TARGETS) *.o *.so ext.kbin libextension.a extension.h .ext)

.PHONY: all clean
