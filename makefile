CC = gcc
CFLAGS = -O3
FILES = src/hashtable.c
OUTPUT_NAME = markov

markov: src/markov.c
	$(CC) -o $(OUTPUT_NAME) src/markov.c $(CFLAGS)

clean:
	del *.exe