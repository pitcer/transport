# Piotr Dobiech 316625

CC = gcc
CFLAGS = -std=gnu17 -Wall -Wextra -Werror -pedantic
LDFLAGS =

BINARY_NAME := transport
SRCS := utils.c time.c io.c window.c transport.c client.c main.c
OBJS := $(SRCS:%.c=%.o)

$(BINARY_NAME): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)

cleandist:
	rm $(OBJS) $(BINARY_NAME)
