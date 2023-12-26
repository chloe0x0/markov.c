#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include <stdint.h>

#define INIT_CAP 1024
#define n 2

/* Default Hash Function 
http://www.cse.yorku.ca/~oz/hash.html
*/
size_t djb2(char* key) {
    size_t hash = 5381;

    int c;
    while (c = *key++) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

size_t crc32b(char *str) {
    // Source: https://stackoverflow.com/a/21001712
    size_t byte, crc, mask;
    int i = 0, j;
    crc = 0xFFFFFFFF;
    while (str[i] != 0) {
        byte = str[i];
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
}

size_t MurmurOAAT64(char* key)
{
  uint64_t h = 525201411107845655ull;
  for (;*key;++key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return (size_t)h;
}

kv* construct_kv(char* key, void* val) {
    kv* pair = malloc(sizeof(kv));
    if (pair == NULL) {
        fprintf(stderr, "Could not allocate space for kv pair");
        exit(EXIT_FAILURE);
    }
    pair->key = key;
    pair->val = val;
    pair->next = NULL;

    return pair;
}

hash_table* build_table() {
    hash_table* table = malloc(sizeof(hash_table));
    if (table == NULL) {
        fprintf(stderr, "Could not allocate space for hashtable"); 
        exit(EXIT_FAILURE);
    }
    table->capacity = INIT_CAP;
    table->size = 0;
    table->kvs = calloc(table->capacity, sizeof(kv*));
    table->hash = &djb2;
    if (table->kvs == NULL) {
        fprintf(stderr, "Could not allocate %d kv pairs\n", INIT_CAP);
        exit(EXIT_FAILURE);
    }

    return table;
}

hash_table* table_with_capacity(size_t cap) {
    hash_table* table = malloc(sizeof(hash_table));
    if (table == NULL) {
        fprintf(stderr, "Could not allocate space for hashtable!");
        exit(EXIT_FAILURE);
    }
    table->capacity = cap;
    table->hash = &djb2;
    table->size = 0;
    table->kvs = calloc(table->capacity, sizeof(kv*));
    if (table->kvs == NULL) {
        fprintf(stderr, "Could not allocate space for hashtable of capacity: %d", cap);
        exit(EXIT_FAILURE);
    }

    return table;
}

bool search(char* val, hash_table* table) {
    size_t hash = table->hash(val) % table->capacity;

    kv* pair = table->kvs[hash];
    if (pair == NULL) return false;

    if (!strcmp(pair->key, val)) return true;

    // search the chain
    kv* search = pair->next;
    if (search == NULL) {
        return false;
    }

    while (search->next != NULL) {
        if (!strcmp(pair->key,val)) return true;
        search = search->next;
    }
    
    return false;
}

bool set(char* key, void* val, hash_table* table) {
    size_t hash = table->hash(key) % table->capacity;
    
    kv* pair = table->kvs[hash];
    while (pair != NULL) {
        if (!strcmp(pair->key, key)) {
            // the key is already present in the hashtable
            pair->val = val;
            return true;
        }

        pair = pair->next;
    }

    kv* new_kv = construct_kv(key, val);
    new_kv->next = table->kvs[hash];
    table->kvs[hash] = new_kv;
}

bool update(char* key, void* new_val, hash_table* table) {
    size_t hash = table->hash(key) % table->capacity;
    kv* pair = table->kvs[hash];
    // element is not in the hashtable
    if (!pair->key) return false; 
    // Check if the element is at the curr kv pair
    if (!strcmp(pair->key,key)) {
        table->kvs[hash]->val = new_val;
        return true;
    }

    kv* search = pair->next;
    if (search == NULL) return false;

    while (search->next != NULL) {
        if (!strcmp(search->key,key)) {
            search->val = new_val;
            return true;
        }
        search = search->next;
    }

    return false;
}

void* get(char* key, hash_table* table) {
    size_t hash = table->hash(key) % table->capacity;
    kv* pair = table->kvs[hash];

    while (pair != NULL) {
        if (!strcmp(pair->key,key)) {
            return pair->val;
        }
        pair = pair->next;
    }

    return NULL;
}

void delete_table(hash_table* table) {
    // need to find ALL nodes, including chained ones
    for (int i = 0; i < table->capacity; i++) {
        if (table->kvs[i] == NULL) {
            continue;
        }
        kv* search = table->kvs[i]->next;
        while (search != NULL) {
            kv* prev = search;
            search = search->next;
            free(prev->key);
            free(prev->val);
            free(prev);
        }
    }
    free(table->kvs);
    free(table);
}

int main(void) {    
    // test collisions on real world data
    FILE* fp = fopen("data/shakespeare.txt", "r");
    if (!fp) {
        printf("Could not open test file!\n");
        exit(EXIT_FAILURE);
    }

    hash_table* freq = table_with_capacity(199973 * 2);
    set_hash(freq, &MurmurOAAT64);

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        // tokenize the line buffer
        char* tokens = strtok(line, " \n\t");
        // use a sliding window of size N, to get N-grams
        while (tokens != NULL) {
            char* n_gram = calloc(sizeof(char), n*100 + 1);
            int i;
            for (i = 0; i < n && tokens!=NULL; i++) {
                strcat(n_gram, tokens);
                tokens = strtok(NULL, " \t\n");
            }
            if (i == n) {
                n_gram[strlen(n_gram) + 1] = '\0';
                if (!search(n_gram,freq)) {
                    set(n_gram,0,freq);
                }
                int prev = ((int)get(n_gram,freq))+1;
                set(n_gram,prev,freq);
            }
        }
    }

    int collisions = 0, elements = 0;

    for (int i = 0; i < freq->capacity; i++) {
        if (freq->kvs[i] == NULL) continue;
        kv* pair = freq->kvs[i];
        elements++;
        printf("freq[%s] = %d\n", pair->key, pair->val);
        kv* search = pair->next;

        while (search != NULL) {
            printf("Chain: freq[%s] = %d\n", search->key, search->val);
            search = search->next;
            collisions++;
            elements++;
        }
    }

    printf("%d collisions with %d elements\n", collisions, elements);

    delete_table(freq);
    // read characters 
    /*
    char c;
    while ((c = fgetc(fp)) != EOF) {
        // do stuff with the character
    }
    */

    // read words/unigrams
    /*
    char c;
    while (c != EOF) {
        // read characters into a string buffer until we find a terminating character
        // whitespace, \n, \t, etc 
        char buffer[50];
        int chars = 0;

        c = fgetc(fp);
        if (is_skip(c)) continue;
        while (c!=EOF && !is_skip(c)) {
            buffer[chars++] = c;
            c = fgetc(fp);
        }
        buffer[chars++] = '\0';
        // do something with buffer
    }*/

    // Generalize to N-grams
    /*
    char c;
    int n = 1;
    while (c != EOF) {
        c = fgetc(fp);
        if (is_skip(c)) continue;
        
        int chars = 0, words = 0;  
        char* buffer = malloc(sizeof(char)*25 * n);

        while (c!=EOF && !is_skip(c)||words<n) {
            buffer[chars++] = c;
            c = fgetc(fp);
            words += is_skip(c);
        }
        buffer[chars] = '\0';
        puts(buffer);

        free(buffer);
    }*/

    // Read N-grams using strtok
    /*
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        // tokenize the line buffer
        char* tokens = strtok(line, " \n\t");
        // use a sliding window of size N, to get N-grams
        while (tokens != NULL) {
            char n_gram[n * 100 + 1] = {0};
            int i;
            for (i = 0; i < n && tokens!=NULL; i++) {
                strcat(n_gram, tokens);
                strcat(n_gram, " ");
                tokens = strtok(NULL, " \t\n");
            }
            n_gram[strlen(n_gram) - 1] = '\0';
            if (i == n) printf("%s\n", n_gram);
        }
    }



    char test[] = "check your window im in your window";
    printf("Computing frequencies of words in: %s\n", test);
    char* word;
    word = strtok(test, " ");
    while (word != NULL) {
        if (!search(word,freq)) {
            set(word,0,freq);
        }
        int prev = ((int)get(word,freq))+1;
        update(word,prev,freq);
        word = strtok(NULL, " ");
    }

    for (int i = 0; i < freq->capacity; i++) {
        if (freq->kvs[i].key == NULL) continue;
        kv pair = freq->kvs[i];
        printf("freq[%s] = %d\n", pair.key, pair.val);
        kv* search = pair.next;
        if (search == NULL) continue;
        while (search->next != NULL) {
            printf("freq[%s] = %d\n", search->key, search->val);
            search = search->next;
        }
    }
    */

    fclose(fp);
}