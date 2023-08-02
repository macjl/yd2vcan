all: clean yd2vcan

yd2vcan: yd2vcan.o
	cc build/yd2vcan.o -o build/yd2vcan -lpthread

yd2vcan.o:
	mkdir -p build
	cc -c main.c -o build/yd2vcan.o -lpthread -Wall

clean:
	rm -f build/*

install:
	cp -a build/yd2vcan /usr/bin/
	cp -a init.d/yd2vcan /etc/init.d/
	cp -a init.d/yd2vcan.conf /etc/yd2vcan.conf
