#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "markov.h"

#define USAGE "markov.exe n-gram-size [text file(s)]"


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
    markov* chain = (markov*)table_with_capacity(19937*2);

    for (int i = 0; i < num_paths; i++) {
        FILE* fp = fopen(paths[i], "r");

        // read the n-grams
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL) {
            // tokenize the line buffer
            char* tokens = strtok(line, " \n\t");
            char* prev_state = "<START>";
            // use a sliding window of size N, to get N-grams
            while (tokens != NULL) {
                char* n_gram = calloc(sizeof(char), N*100 + 1);
                int i;
                for (i = 0; i < N && tokens!=NULL; i++) {
                    strcat(n_gram, tokens);
                    strcat(n_gram, " ");
                    tokens = strtok(NULL, " \t\n");
                }
                if (i == N) {
                    n_gram[strlen(n_gram) + 1] = '\0';
                    if (!search(prev_state,chain)) {
                        hash_table* trans = table_with_capacity(15);
                        set(prev_state,(void*)trans,chain);
                    }
                    hash_table* trans = (hash_table*)get(prev_state,chain);
                    if (!search(n_gram,trans)) {
                        set(n_gram,0,trans);
                    }
                    int prev = ((int)get(n_gram,trans)) + 1;
                    update(n_gram,prev,trans);
                    prev_state = n_gram;
                }
            }
        }

        fclose(fp);
    }

    return chain;
}

char* sample(markov* chain, char* state) {
    hash_table* trans = get(state,chain);
    if (trans == NULL) {
        return NULL;
    }

    uint32_t sum = 0, acc = 0;
    for (int i = 0; i < trans->capacity; i++) {
        if (trans->kvs[i] == NULL) continue;
        kv* curr = trans->kvs[i];
 
        while (curr != NULL) {
            sum += (int)curr->val;
            curr = curr->next;
        }
    }

    size_t r = rand() % sum;

    char* curr_best = NULL;

    for (int i = 0; i < trans->capacity; i++) {
        if (trans->kvs[i] == NULL) continue;

        kv* curr = trans->kvs[i];
        while (curr != NULL) {
            acc += (int)curr->val;
            if (r < acc) {
                return curr->key;
            }

            curr_best = curr->key;
            curr = curr->next;
        }
    }

    return curr_best;
}

char* gen(markov* chain, char* state, uint32_t N) {
    char* seq = malloc(sizeof(char)*N+100);
    char* curr = state;

    for (int i = 0; i < N; i++) {
        strcat(seq, curr);
        curr = sample(chain,curr);
        if (curr == NULL) break;
    }

    return seq;
}

void destroy_markov(markov* chain) {
    for (int i = 0; i < chain->capacity; i++) {
        kv* curr = chain->kvs[i];
        if (curr == NULL) continue;
        while (curr) {
            free(curr->key);
            delete_table(curr->val);
            curr = curr->next;
        }
    }

    free(chain->kvs);
    free(chain);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        //puts(USAGE);
        //exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    const char* paths[1] = {"data/ts.txt"};
    markov* chain = ngram_fit(paths, 1, 2);
    char* text = gen(chain, "Jang had ", 25);
    printf("%s\n", text);
    free(text);
    destroy_markov(&chain);
}