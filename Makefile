all: clean yd2vcan

yd2vcan: yd2vcan.o
	cc yd2vcan.o -o yd2vcan -lpthread

yd2vcan.o:
	cc -c main.c -o yd2vcan.o -lpthread -Wall

clean:
	rm -f yd2vcan yd2vcan.o
