savedcmd_greeter.o := ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o greeter.o @greeter.mod  ; /home/user/Symbi-OS/linux/tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --ibt --orc --retpoline --rethunk --sls --static-call --uaccess --prefix=16  --link  --module greeter.o

greeter.o: $(wildcard /home/user/Symbi-OS/linux/tools/objtool/objtool)
