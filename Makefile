CC=gcc
CFLAGS=-O2 -g3 -Wall
LDFLAGS=-lusb

TARGET=gpib_console
OBJS=gpib_console.o

.PHONY: all clean

all:		$(TARGET)

clean:
		rm -f $(TARGET) $(OBJS)

$(TARGET):	$(OBJS)
		$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

