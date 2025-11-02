CC=gcc
CFLAGS=-Wall $(shell pkg-config --cflags dbus-1)
LDFLAGS=-l asound -lpulse -ldbus-1

status: status.o network.o battery.o volume.o bluetooth.o
	$(CC) $(LDFLAGS) status.o network.o battery.o volume.o bluetooth.o -o status

status.o: status.c
	$(CC) $(CFLAGS) -c status.c -o status.o

network.o: network.c
	$(CC) $(CFLAGS) -c network.c -o network.o

bluetooth.o: bluetooth.c
	$(CC) $(CFLAGS) -c bluetooth.c -o bluetooth.o

battery.o: battery.c
	$(CC) $(CFLAGS) -c battery.c -o battery.o

volume.o: volume.c
	$(CC) $(CFLAGS) -c volume.c -o volume.o

clean:
	rm -f status status.o network.o battery.o volume.o bluetooth.o

install: status
	cp ./status /usr/local/bin/status

uninstall:
	rm -f /usr/local/bin/status
