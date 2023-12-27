CC = gcc
CFLAGS = -O3
FILES = src/hashtable.c
OUTPUT_NAME = markov

markov: src/markov.c
	$(CC) -o $(OUTPUT_NAME) src/markov.c $(CFLAGS)

hashtable_test: src/hashtable.c src/hashtable.h
	$(CC) -o test src/hashtable.c

clean:
	rm *.exe