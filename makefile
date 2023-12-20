CC = gcc
CFLAGS = -O3
OUTPUT_NAME = markov

markov: src/markov.c
	$(CC) -o $(OUTPUT_NAME) src/markov.c $(CFLAGS)

clean:
	del $(OUTPUT_NAME).exe