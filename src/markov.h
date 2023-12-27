#ifndef MARKOV_H_
#include "hashtable.h"
#include <stdint.h>

typedef hash_table markov_chain;
typedef uint32_t** char_markov_chain;

// Fit a markov chain to text where each state is a character
char_markov_chain character_fit(const char**, int);
// Fit a markov chain to text where each state is an ngram
markov_chain* ngram_fit(const char**, int, int);
// Fit a markov chain to text where each state is arbitrary
markov_chain* fit(const char**, int, int);
// Read n-grams from text
char** read_ngrams(const char*, int, int);
// Destructor for a char_markov_chain
void destroy_char_markov_chain(char_markov_chain*);

#endif /* End of MARKOV_H_ implementation */