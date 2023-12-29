#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hashtable.h"

#define INIT_CAP 1024
#define REHASH_THRESH 0.75
#define GROWTH_FACTOR 2
#define SEED 19337

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

/* DEFAULT HASH FUNCTION */
size_t MurmurOAAT64(char* key) {
    const uint64_t len = strlen(key);
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = SEED ^ (len * m);
    const uint64_t *data = (const uint64_t*)key;
    const uint64_t* end = data + len/8;

    while (data != end) {
        uint64_t k = *data++;
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }
    
    const uint8_t* tail = (const uint8_t*)data;
    uint64_t k = 0;

    switch (len & 7) {
        case 7: k ^= tail[6] << 48;
        case 6: k ^= tail[5] << 40;
        case 5: k ^= tail[4] << 32;
        case 4: k ^= tail[3] << 24;
        case 3: k ^= tail[2] << 16;
        case 2: k ^= tail[1] << 8;
        case 1: k ^= tail[0];
                k *= m;
    }

    h ^= k;
    h ^= h >> r;
    h *= m;
    h ^= h >> r;

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
    table->hash = &MurmurOAAT64;
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

void set_hash(hash_table* table, size_t (*f)(char*)) {
    table->hash = f;
}

bool search(char* key, hash_table* table) {
    size_t hash = table->hash(key) % table->capacity;

    kv* pair = table->kvs[hash];
    while (pair != NULL) {
        if (!strcmp(pair->key,key)) {
            return true;
        }
        pair = pair->next;
    }
    
    return false;
}

void rehash(hash_table* table) {
    hash_table new_table = {
        // new kvs
        calloc(table->capacity*GROWTH_FACTOR, sizeof(kv*)),
        // new capacity
        table->capacity*GROWTH_FACTOR,
        // new size
        0,
        // new hash function
        table->hash
    };

    for (int i = 0; i < table->capacity; i++) {
        if (table->kvs[i] == NULL) continue;
        // Need to rehash and insert into new table
        kv* curr = table->kvs[i];
        while (curr != NULL) {
            kv* nxt = curr->next;
            size_t idx = new_table.hash(curr->key) % new_table.capacity;
            curr->next = new_table.kvs[idx];
            new_table.kvs[idx] = curr;
            new_table.size++;
            curr = nxt;
        }
    }

    free(table->kvs);
    *table = new_table;
}

bool set(char* key, void* val, hash_table* table) {
    // check if we need to rehash
    // by computing the ~load~factor >w<
    if (table->size == table->capacity*REHASH_THRESH) {
        // rehash
        rehash(table);
    }

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

    // insert the new kv pair
    kv* new_kv = construct_kv(key, val);
    new_kv->next = table->kvs[hash];
    table->kvs[hash] = new_kv;
    table->size++;
}

bool update(char* key, void* new_val, hash_table* table) {
    size_t hash = table->hash(key) % table->capacity;
    kv* pair = table->kvs[hash];

    while (pair != NULL) {
        if (!strcmp(pair->key,key)) {
            pair->val = new_val;
            return true;
        }
        pair = pair->next;
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
            kv* prev = search->next;
            free(search->key);
            free(search->val);
            free(search);
            search = prev;
        }
    }
    free(table->kvs);
    free(table);
}

/*
int main(void) {    
    // test collisions on real world data
    FILE* fp = fopen("data/ts.txt", "r");
    if (!fp) {
        printf("Could not open test file!\n");
        exit(EXIT_FAILURE);
    }

    hash_table* freq = build_table();

    #define n 2
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
                strcat(n_gram, " ");
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
        //printf("freq[%s] = %d\n", pair->key, pair->val);
        kv* search = pair->next;

        while (search != NULL) {
            //printf("Chain: freq[%s] = %d\n", search->key, search->val);
            search = search->next;
            collisions++;
            elements++;
        }
    }

    printf("%d collisions with %d elements\n", collisions, elements);
    printf("%d elements, capacity of %d buckets\n", freq->size, freq->capacity);
    printf("Load factor: %f\n", (float)freq->size/(float)freq->capacity);
    

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

    fclose(fp);
}
*/