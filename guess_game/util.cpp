#include "util.h"
#include <string.h>

unsigned int defaultHashFunction(const void* key)
{
    return *(unsigned int*)key;
}

int defaultKeyCompare(const void* key1, const void* key2)
{
    return (*(unsigned int*)key1 == *(unsigned int*)key2);
}

unsigned int nameHashCode(const void* key)
{
    //将昵称ascii相加作hashkey
    const char* name = (const char*)key;
    int nlen = strlen(name);
    int ascii_sum = 0;
    for (int i = 0; i < nlen; ++i)
        ascii_sum += name[i];
    return ascii_sum;
}

int nameKeyCompare(const void* key1, const void* key2)
{
    //FIXME:more efficient
    int l1 = strlen((const char*)key1);
    int l2 = strlen((const char*)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

HashTable* createHashTable(const unsigned int size, const HashFunction hash_func, const KeyCompare compare_func)
{
    //初始化结构
    HashTable* table = new HashTable();
    table->size = size;
    table->sizemask = table->size - 1;
    table->buckets = (ListNode**)calloc(1, sizeof(ListNode*) * size);
    if (hash_func != NULL) table->hashFunc = hash_func;
    if (compare_func != NULL) table->keyCompare = compare_func;

    //哨兵首结点
    table->head = new ListNode();
    return table;
}