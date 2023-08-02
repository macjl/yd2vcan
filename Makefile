all: clean yd2vcan

yd2vcan: yd2vcan.o
	cc build/yd2vcan.o -o build/yd2vcan -lpthread

yd2vcan.o:
	mkdir -p build
	cc -c main.c -o build/yd2vcan.o -lpthread -Wall

clean:
	rm -f build/*
	rm -f *~

install:
	cp -a build/yd2vcan /usr/bin/
	cp -a yd2vcan.sh /usr/bin
	[ ! -e /etc/yd2vcan.conf ] && cp -a yd2vcan.conf /etc/yd2vcan.conf || exit 0
