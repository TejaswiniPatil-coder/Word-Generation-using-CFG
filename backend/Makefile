CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
TARGET = cfg_sim

SRCS = main.c grammar.c generator.c parser.c tree.c utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)
	@echo "Build successful! Run: ./$(TARGET)"

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
