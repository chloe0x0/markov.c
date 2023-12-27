#ifndef HASHTABLE_H_

/* 
    How to resolve collisions?
    Seperate Chaining vs Open Addressing?
    Lets use Seperate Chaining

    If character level markov chain,
    the number of entries needed is ~130

    If N-gram, the number of entires could be precomputed
    Use W - N + 1 entries
    Question: Number of unique N-grams in a text with W words.

    "How to resolve collisions", W = 4
    let N = 2,
    "How to", "to resolve", "resolve collisions"
    N = 3,
    "How to resolve", "to resolve collisions"
    Looks like W - N + 1 works

    Going to try to make it such that we never have to rehash
    
*/

typedef struct kv_t {
    char* key;  // can be an n-gram or a single character
    void* val;  // Pointer to the transition list

    struct kv_t* next; // Pointer to the chain
} kv;

typedef struct hash_table_t {
    kv** kvs;
    size_t capacity, size;
    // the hash function
    size_t (*hash) (char*);
} hash_table;

// Insert a kv pair into a hashtable
bool set(char*, void*, hash_table*);
// Returns true if an element is in the hashtable keys
bool search(char*, hash_table*);
// Update a kv pair in a hashtable
bool update(char*, void*, hash_table*);
// Get a val from a hash table given its key
void* get(char*, hash_table*);
// Constructor for a hashtable with the default capacity
hash_table* build_table();
// Constructor for a hashtable with a specified capacity
hash_table* table_with_capacity(size_t);
// Change the hash function of a hashtable
void set_hash(hash_table* table, size_t (*f)(char*)) {table->hash = f;}
// Destructor for a hash table
void delete_table(hash_table*);

#endif /* End of HASHTABLE_H_ implementation */