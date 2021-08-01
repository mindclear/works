#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>

//接口定义
typedef unsigned int (*HashFunction)(const void* key);  //计算hash值的函数
typedef int (*KeyCompare)(const void* key1, const void* key2); //比较key的函数

//默认hash函数
unsigned int defaultHashFunction(const void* key);
int defaultKeyCompare(const void* key1, const void* key2);
//字符串hash
unsigned int nameHashCode(const void* key);
int nameKeyCompare(const void* key1, const void* key2);

struct ListNode
{
    void* key;
    void* val;
    unsigned int hash;
    ListNode* pre;
    ListNode* next;
    ListNode* hnext;

    ListNode()
        :key(NULL), val(NULL), hash(0), pre(NULL), next(NULL), hnext(NULL)
    {}
};

struct HashTable
{
    ListNode* head;
    ListNode** buckets;
    unsigned int size;
    unsigned int sizemask;
    unsigned int used;
    HashFunction hashFunc;
    KeyCompare keyCompare;

    HashTable()
        :head(NULL), buckets(NULL), size(0), sizemask(0), used(0), hashFunc(defaultHashFunction), keyCompare(defaultKeyCompare)
    {}

    ~HashTable()
    {
        ListNode* cur = head;
        while (cur)
        {
            ListNode* next = cur->next;
            delete cur;
            cur = next;
        }
        //FIXME:统一new\free
        delete head;
        free(buckets);
    }

    ListNode* Head()
    {
        return head->next;
    }

    void* Get(const void* key)
    {
        unsigned int hash = hashFunc(key);
        ListNode* cur = buckets[hash&sizemask];
        if (NULL == cur)
            return NULL;
        
        while (cur != NULL)
        {
            if (hash == cur->hash && keyCompare(key, cur->key))
                return cur->val;
            cur = cur->hnext;
        }
        return NULL;
    }

    void Set(void* key, void* val)
    {
        unsigned int hash = hashFunc(key);
        unsigned int idx = (hash&sizemask);
        ListNode* cur = buckets[idx];
        while (cur != NULL)
        {
            if (hash == cur->hash && keyCompare(key, cur->key))
            {
                cur->val = val;
                return;
            }
            cur = cur->hnext;
        }

        //插入hash链表
        //FIXME:memecpy key
        ListNode* tmp = new ListNode();
        tmp->key = key;
        tmp->val = val;
        tmp->hash = hashFunc(key);
        tmp->hnext = buckets[idx];
        buckets[idx] = tmp;

        //插入顺序链表
        if (head->next != NULL)
            head->next->pre = tmp;
        tmp->next = head->next;
        tmp->pre = head;
        head->next = tmp;
    }

    void Del(void* key)
    {
        unsigned int hash = hashFunc(key);
        unsigned int idx = (hash&sizemask);
        ListNode* cur = buckets[idx];
        if (NULL == cur)
            return;
        
        ListNode* pre = NULL;
        while (cur != NULL)
        {
            if (hash == cur->hash && keyCompare(key, cur->key))
            {
                if (pre != NULL) 
                    pre->hnext = cur->hnext;
                else
                    buckets[idx] = cur->hnext;

                if (cur->pre != NULL) cur->pre->next = cur->next;
                if (cur->next != NULL) cur->next->pre = cur->pre;
                return;
            }
            pre = cur;
            cur = cur->hnext;
        }
    }
};

//创建HashTable
HashTable* createHashTable(const unsigned int size, const HashFunction hash_func, const KeyCompare compare_func);

#endif //__UTIL_H__
