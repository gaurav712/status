CC="gcc"
CFLAGS="-Wall"

all:
	$(CC) -o status $(CFLAGS) status.c

clean:
	rm -f status

install: all
	cp status /usr/local/bin/status

uninstall:
	rm -f /usr/local/bin/status
