#ifndef MARKOV_H_
#include "hashtable.h"
#include <stdbool.h>

typedef struct markov_chain {
    hash_table transitions;
} markov_chain;

#endif /* End of MARKOV_H_ implementation */