#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
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

cmarkov character_fit(const char** paths, int num_paths) {
    cmarkov chain = calloc(256,sizeof(uint32_t*));    
    for (short i = 0; i < 256; i++) chain[i] = calloc(256,sizeof(uint32_t));

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

char csample(cmarkov* chain, char state) {
    // get transition list
    uint32_t* trans = (*chain)[state];

    // perform random weighted sampling
    // we know ahead of time that the number of elements is 256
    // compute the sum
    uint32_t sum = 0;
    for (int i = 0; i < 256; i++) sum += trans[i];
    size_t r = rand() % sum, res = 0, acc = 0;
    for (int i = 0; i < 256; i++) {
        acc += trans[i];
        if (r < acc) {
            res = i;
            break;
        }
    }
    return (char)res;
}

char* cgen(cmarkov* chain, uint32_t N, char init) {
    char* seq = malloc(sizeof(char)*(N+2));

    char prev = init;
    for (int i = 0; i < N; i++) {
        seq[i] = prev;
        prev = csample(chain, prev);
    }

    seq[N+2] = '\0';
    return seq;
}

void destroy_cmarkov(cmarkov* chain) {
    for (int i = 0; i < 256; i++) free(chain[i]);
    free(chain); 
}

markov* ngram_fit(const char** paths, uint32_t num_paths, uint32_t N) {
    markov* chain = (markov*)table_with_capacity(100);

    for (int i = 0; i < num_paths; i++) {
        FILE* fp = fopen(paths[i], "r");

        // read the n-grams


        fclose(fp);
    }

    return chain;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        //puts(USAGE);
        //exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    const char* paths[1] = {"data/ts.txt"};
    cmarkov chain = character_fit(paths, 1);
    char* text = cgen(&chain, 1000, 'T');
    printf("%s\n", text);
    free(text);
    destroy_cmarkov(&chain);
}