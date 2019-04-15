CC=gcc
CFLAGS=-lwiringPi -lm -lasound
DEPS=src/servo.h src/utils.h
OBJ=src/locator.o src/servo.o src/utils.o

TESTDEPS=test/minunit.h ../src/utils.h ../src/servo.h ../src/utils.c ../src/servo.c
TESTSRC=test/runtests.c
TESTOBJ=test/runtest.o
TESTFLAGS=-lwiringPi -lm

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

locator: $(OBJ)
	gcc $(CFLAGS) -o $@ $^

runtests: $(TESTSRC)
	gcc -o $@ $^ $(TESTFLAGS)

test: runtests
	sudo ./runtests

clean :
	rm $(OBJ)

cleantest:
	rm $(TESTOBJ)



