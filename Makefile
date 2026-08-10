CC := cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror
TARGET := hash_table
SOURCES := src/main.c src/hash_table.c src/prime.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -f $(TARGET)
