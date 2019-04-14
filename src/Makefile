CC=gcc
CFLAGS= -lwiringPi -lm -lasound

TARGET = locator
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
