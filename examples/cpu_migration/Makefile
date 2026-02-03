CC = gcc

# lets go through these and make sure we understand what we really need -- if dpld logic
# requires mitigations then let's stack switch extra as needed
CFLAGS = -O0 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer -Wfatal-errors \
	-fPIC
CFLAGS += -I ../../Symlib/include -DDYNLINK

TARGETS :=  main main_static
.PHONY: all clean libkallsyms.so libkallsyms.a

all: $(TARGETS)

# extension boiler plate
libextension.a extension.h ext.kbin: $(wildcard *.kc *.kh)
	../tools/kcc

libkallsyms.so:
	cp /proc/$@ $@

libkallsyms.a:
	cp /proc/$@ $@

# main program
main.o: main.c extension.h 
	$(CC) $(CFLAGS) -c -o main.o main.c

main:  main.o libextension.a libkallsyms.so
#main:  main.o libkallsyms.so
#	$(CC) $(CFLAGS) -o $@ $< -lc -L../../Symlib/dynam_build -lSym -L. -lkallsyms
	$(CC) $(CFLAGS) -o $@ $< -lc ../../Symlib/build/L0/L0.a -L. -lkallsyms -lextension 

main_static: main.o libkallsyms.a libextension.a
	$(CC) $(CFLAGS) -o $@ $^  libkallsyms.a libextension.a -lc -L../../Symlib/dynam_build -lSym  

clean:
	rm -rf $(wildcard $(TARGETS) *.o *.so *.a ext.kbin  extension.h .ext)


