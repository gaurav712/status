CC=gcc
CFLAGS=-l asound -Wall

status: status.o network.o battery.o volume.o
	$(CC) $(CFLAGS) status.o network.o battery.o volume.o -o status

status.o: status.c
	$(CC) -c status.c -o status.o

network.o: network.c
	$(CC) -c network.c -o network.o

battery.o: battery.c
	$(CC) -c battery.c -o battery.o

volume.o: volume.c
	$(CC) -c volume.c -o volume.o

clean:
	rm -f status status.o network.o battery.o volume.o

install:
	cp ./status /usr/local/bin/status

uninstall:
	rm -f /usr/local/bin/status