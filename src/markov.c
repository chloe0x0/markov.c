#include <stdio.h>
#include <stdlib.h>

#define USAGE "markov.exe n-gram-size [text file(s)]"

/*
        How to represent the Markov Chain?
        How to get N-Grams?
        What capabilities do we want?

        Want to allow character and n-gram level Markov Chains
        Serializable models
        Fast generation
            Generate to stdout or to a FILE*
        
*/

int main(int argc, char** argv) {
    if (argc < 2) {
        puts(USAGE);
        exit(EXIT_FAILURE);
    }
}