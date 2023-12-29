#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "markov.h"

#define MAX_FILES 900
#define USAGE "markov.exe is-char[bool] order[int] iters(int) init_state[string] [text files]"

bool arg_to_bool(const char* arg) {
    if (!strcmp(arg,"0")) { return false; }
    return true;
}

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
    if (trans == NULL) {
        fprintf(stderr, "%c not in chain!\n", state);
        exit(EXIT_FAILURE);
    }

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
        char* prev_state = "<START>";
        while (fgets(line, sizeof(line), fp) != NULL) {
            // tokenize the line buffer
            char* tokens = strtok(line, " \n\t");
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

markov* fit(const char** paths, uint32_t num_paths, uint32_t order, bool is_char) {
    if (!is_char) return ngram_fit(paths, num_paths, order);

    markov* chain = table_with_capacity(19937 * 2);

    // fit by N character model
    for (int i = 0; i < num_paths; i++) {
        FILE* fp = fopen(paths[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "Could not read: %s\n", paths[i]);
            exit(EXIT_FAILURE);
        }

        char* prev_state = "<START>";
        char c = '\0';
        while (c != EOF) {
            char* state = calloc(sizeof(char),order+2);
            int j;
            for (j = 0; j < order && c != EOF; j++) {
                c = fgetc(fp);
                state[j] = c;
            }
            state[order + 1] = '\0';

            if (!search(prev_state, chain)) {
                hash_table* trans = table_with_capacity(257);
                set(prev_state, trans, chain);
            }
            hash_table* trans = get(prev_state, chain);

            if (!search(state, trans)) {
                set(state,0,trans);
            }

            int prev = (int)get(state,trans) + 1;
            update(state, prev, trans);
            prev_state = state;
        }
        fclose(fp);
    }

    return chain;
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
        puts(USAGE);
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    // shift the args to remove the executable name
    *argv++; 

    // parse out the is-char
    bool is_char = arg_to_bool(*argv++);
    // parse out the order
    int order = atoi(*argv);
    if (order == 0) {
        if (is_char) fprintf(stderr, "Invalid number of characters [%s] per state!\n", *argv);
        else fprintf(stderr, "Invalid number of words [%s] per state!\n", *argv);
    }
    argv++;

    // Parse out the iters
    int iters = atoi(*argv);
    if (iters == 0) {
        fprintf(stderr, "Invalid number of generated states!\n Expected an integral number, got: %s\n", *argv);
    }

    argv++;

    // Parse out the initial state
    char* init_state = *argv++;

    // get the text files
    const char** paths = malloc(sizeof(char*)*MAX_FILES);
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (i >= argc-5) {
            break;
        }
        paths[i] = *argv++;
    }

    printf("is_char: %d, iters: %d, order: %d, files: %i, init_state: %s\n", is_char, iters, order, i, init_state);

    if (is_char && order == 1) {
        cmarkov chain = character_fit(paths, i);
        char* text = cgen(&chain, iters, *init_state);
        printf("%s\n", text);
        free(text);
        destroy_cmarkov(&chain);
        exit(EXIT_SUCCESS);
    }

    markov* chain = fit(paths, i, order, is_char);
    char* text = gen(chain, init_state, iters);
    printf("%s\n", text);
    free(text);
    destroy_markov(chain);
}