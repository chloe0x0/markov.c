#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "markov.h"

#define USAGE "markov.exe n-gram-size [text file(s)]"
/*
        How to represent the Markov Chain?
            Graph Representation
            Adjacency List (weighted)

        Hashtable<String,Something>

        How to get N-Grams?
        What capabilities do we want?

        Want to allow character and n-gram level Markov Chains
        Serializable models
        Fast generation
            Generate to stdout or to a FILE*
        

        Markov Chain structure:

        Hash Table mapping states to their state transition lists
        Transition lists: Hashtables mapping connected states and their transition probabilities
            Why store transition lists as hashtables? We are always doing a linear traversal over them anyway
            Instead, use a dynamic array or linked list

*/

char_markov_chain character_fit(const char** paths, int num_paths) {
    char_markov_chain chain = calloc(256,sizeof(uint32_t*));    
    for (int i = 0; i < 256; i++) chain[i] = calloc(256,sizeof(uint32_t));

    // iterate over the filepaths 
    for (int i = 0; i < num_paths; i++) {
        // Read the characters
        FILE* fp = fopen(paths[i], "r");
        char c = fgetc(fp), prev = '\0';
        while (c != EOF) {
            prev = c;
            c = fgetc(fp);
            chain[prev][c]++;
        }

        fclose(fp);
    }

    return chain;
}

void destroy_char_markov_chain(char_markov_chain* chain) {
    for (int i = 0; i < 256; i++) free(chain[i]);
    free(chain);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        //puts(USAGE);
        //exit(EXIT_FAILURE);
    }
    const char* paths[1] = {"data/shakespeare.txt"};
    char_markov_chain chain = character_fit(paths, 1);

    char c = 'a';
    uint32_t* transitions = chain[c];
    for (int i = 0; i < 256; i++) {
        printf("a -> %c | %d\n", (char)i, chain[c][i]);
    }

    destroy_char_markov_chain(&chain);
}