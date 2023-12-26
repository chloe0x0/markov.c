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

int main(int argc, char** argv) {
    if (argc < 2) {
        puts(USAGE);
        exit(EXIT_FAILURE);
    }
}