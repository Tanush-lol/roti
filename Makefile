CC = gcc
CFLAGS = -Wall -Wextra -g 
LDFLAGS = -lwayland-client 

TARGET = gui 

SRCS = src/wayland-client.c 

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean
