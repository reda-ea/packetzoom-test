
#include <malloc.h>
#include <stdio.h>
#include <string.h>

struct keydata {
    char name[50];
    unsigned long sum;
    unsigned freq;
};

// for simplicity, a store is an array of keydata structs
// the first item of the array is reserved to the size of the store
// we also assume we allocated enough memory for the store
// we can improve later by dynamically allocating / expanding memory, etc
// OR we could move to an STL data structure and avoid this poblem altogether

struct keydata *new_store() {
    struct keydata *store = malloc(1000 * sizeof(struct keydata));
    store[0].name[0] = 0;
    store[0].sum = 0;
    store[0].freq = 0;
    return store;
}

void add_key(struct keydata *store, char *key, unsigned long value) {
    int storelen = store[0].sum;
    int i;
    for(i = 1; i < storelen + 1; ++i)
        if(!strncmp(store[i].name, key, 49)) {
            store[i].sum += value;
            store[i].freq += 1;
            return;
        }
    strncpy(store[storelen + 1].name, key, 49);
    store[storelen + 1].sum = value;
    store[storelen + 1].freq = 1;
    store[0].sum = storelen + 1;
    return;
}

int extract_keyvalue(char *key, char *value, char *line) {
    char *pos = line;
    for(; *pos && *pos != '"'; ++pos);
    if(!*pos)
        return 0;
    ++pos;
    int i = 0;
    for(; *pos && *pos != '='; ++pos)
        key[i++] = *pos;
    if(!*pos)
        return 0;
    key[i] = 0;
    ++pos;
    i = 0;
    for(; *pos && *pos != '"'; ++pos)
        value[i++] = *pos;
    if(!*pos)
        return 0;
    value[i] = 0;
    return 1;
}

// int main() {
//     char k[100];
//     char v[100];
//     extract_keyvalue(k, v, "something something \"mykey=125486y652\"kjh;kj");
//     printf("%s => %s\n", k, v);
// }

int pipetostore(struct keydata *store, int pipefd, void (*callback)(struct keydata *)) {
    char buffer[1024];
    char k[100];
    char v[100];
    int pos = 0;
    while(read(pipefd, buffer + pos, 1) > 0)
        if(buffer[pos] == 0) {
            int has_kv = extract_keyvalue(k, v, buffer);
            if(has_kv) {
                add_key(store, k, atol(v));
                callback(store);
            }
            pos = 0;
        } else
            pos += 1;
}

void dump_store(struct keydata *store) {
    int storelen = store[0].sum;
    int i = 1;
    for(i=1; i < storelen + 1; ++i)
        printf("%s : %lu : %d\n", store[i].name, store[i].sum, store[i].freq);
    printf("=============================\n");
}

// int main() {
//     struct keydata *store = new_store();
//     add_key(store, "test1", 256);
//     add_key(store, "reda", 5);
//     add_key(store, "test1", 2);
//     add_key(store, "buffer", 0);
//     add_key(store, "reda", 5);
//     add_key(store, "test1", 1);
//     dump_store(store);
//     free(store);
// }
