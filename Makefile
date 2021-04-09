CC=gcc
CFLAGS=-l asound -Wall
USER=$(shell whoami)

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

install: status
	cp ./status /usr/local/bin/status

install_font:
	[[ $(USER) == "root" ]] && cp ./font/feather.ttf /usr/share/fonts/feather.ttf || mkdir -p ~/.local/share/fonts && cp ./font/feather.ttf ~/.local/share/fonts/feather.ttf

uninstall:
	rm -f /usr/local/bin/status
