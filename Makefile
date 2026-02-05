CFLAGS+=-Wall -Wextra -I.
VERSIONFILE=./VERSION

unspace: unspace.o cli.o utils.o

unspace.o: unspace.c $(VERSIONFILE)
unspace.o: CFLAGS+='-DUNSPACE_VERSION="$(shell cat $(VERSIONFILE))"'
