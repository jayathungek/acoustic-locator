CC=gcc
CFLAGS=-lwiringPi -lm -lasound
DEPS=src/servo.h src/utils.h
OBJ=src/locator.o src/servo.o src/utils.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

locator: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
