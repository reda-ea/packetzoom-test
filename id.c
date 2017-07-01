
#include <stdio.h>
#include <sys/time.h>

// http://www.cse.yorku.ca/~oz/hash.html
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

int stouid(char *str) {
    return itouid(hash(str));
}

// https://gist.github.com/shabinesh/2403356
int itouid(unsigned long u_id)
{
    struct timeval t;
    unsigned long id;
    gettimeofday(&t,NULL);
    id = (t.tv_sec * 1000 * 1000) + (t.tv_usec * 1000) << 42;
    id |= (u_id % 16777216) << 24;
    return id;
}
