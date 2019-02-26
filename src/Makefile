CC=gcc
CFLAGS= -lwiringPi -lpthread

TARGET = mux
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
