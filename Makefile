CFLAGS+=-Wall -Wextra -I. -std=gnu23
VERSIONFILE=./VERSION

unspace: unspace.o cli.o utils.o

unspace.o: unspace.c $(VERSIONFILE)
unspace.o: CFLAGS+='-DUNSPACE_VERSION="$(shell cat $(VERSIONFILE))"'
