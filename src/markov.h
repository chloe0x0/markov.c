#ifndef MARKOV_H_
#include "hashtable.h"
#include <stdint.h>

typedef hash_table markov;
typedef uint32_t** cmarkov;

// Fit a markov chain to text where each state is a character
cmarkov character_fit(const char**, int);
// Fit a markov chain to text where each state is an ngram
markov* ngram_fit(const char**, int, int);
// Fit a markov chain to text where each state is arbitrary
markov* fit(const char**, int, int);
// Read n-grams from text
char** read_ngrams(const char*, int, int);
// Destructor for a char_markov_chain
void destroy_cmarkov(cmarkov*);
// Sample from a char_markov_chain
char csample(cmarkov*,char);

#endif /* End of MARKOV_H_ implementation */