CC=gcc
CFLAGS=-lwiringPi -lm -lasound
DEPS=servo.h utils.h
OBJ=locator.o servo.o utils.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

locator: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
