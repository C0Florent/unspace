CFLAGS+=-Wall -Wextra -I.

unspace: unspace.o cli.o utils.o

unspace.o: unspace.c VERSION
unspace.o: CFLAGS+= '-DUNSPACE_VERSION="$(shell cat ./VERSION)"'
