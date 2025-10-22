CC=gcc
CFLAGS=-l asound -Wall

status: status.o network.o battery.o volume.o bluetooth.o
	$(CC) $(CFLAGS) status.o network.o battery.o volume.o bluetooth.o -o status

status.o: status.c
	$(CC) -c status.c -o status.o

network.o: network.c
	$(CC) -c network.c -o network.o

bluetooth.o: bluetooth.c
	$(CC) -c bluetooth.c -o bluetooth.o

battery.o: battery.c
	$(CC) -c battery.c -o battery.o

volume.o: volume.c
	$(CC) -c volume.c -o volume.o

clean:
	rm -f status status.o network.o battery.o volume.o bluetooth.o

install: status
	cp ./status /usr/local/bin/status

uninstall:
	rm -f /usr/local/bin/status
