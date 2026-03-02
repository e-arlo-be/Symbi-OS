savedcmd_greeter.mod := printf '%s\n'   interpos.o main.o | awk '!x[$$0]++ { print("./"$$0) }' > greeter.mod
