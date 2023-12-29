CC = gcc
CFLAGS = -O3 -ffast-math
FILES = src/hashtable.c
OUTPUT_NAME = markov

hashtable.o: src/hashtable.c src/hashtable.h
	$(CC) -c src/hashtable.c

markov: src/markov.c src/hashtable.o
	$(CC) $(CFLAGS) -o $(OUTPUT_NAME) src/markov.c $(FILES)

hashtable_test: src/hashtable.c src/hashtable.h
	$(CC) -o test src/hashtable.c

clean:
	rm *.exe