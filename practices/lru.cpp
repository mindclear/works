#include <stdio.h>

#define HashKey(key) key

struct ListNode
{
    int key;
    int value;
    ListNode* pre;
    ListNode* next;
    ListNode* hnext;

    ListNode()
        :key(0), value(0), pre(NULL), next(NULL), hnext(NULL)
    {}
};

struct HashTable
{
    ListNode* head;
    ListNode* tail;
    ListNode** buckets;
    unsigned int size;
    unsigned int sizemask;
    unsigned int used;

    HashTable()
        :head(NULL), tail(NULL), buckets(NULL), size(0), sizemask(0), used(0)
    {}

    ListNode* Get(const int key)
    {
        unsigned int idx = HashKey(key) & sizemask;
        ListNode* cur = buckets[idx];
        while (cur != NULL)
        {
            if (key == cur->key)
            {
                Update(cur);
                return cur;
            }
            cur = cur->hnext;
        }
        return NULL;
    }

    void Set(const int key, int val)
    {
        unsigned int idx = HashKey(key) & sizemask;
        ListNode* cur = buckets[idx];
        ListNode* pre = NULL;
        while (cur != NULL)
        {
            if (key == cur->key)
            {
                cur->value = val;
                Update(cur);
                return;
            }
            pre = cur;
            cur = cur->hnext;
        }

        ListNode* tmp = new ListNode();
        tmp->key = key;
        tmp->value = val;
        if (pre)
            pre->hnext = tmp;
        else
            buckets[idx] = tmp;
        Update(tmp);
    }

    void Update(ListNode* cur)
    {
        //从当前位置拿出来
        if (cur->pre != NULL)
            cur->pre->next = cur->next;
        if (cur->next != NULL)
            cur->next->pre = cur->pre;

        //插入到首位
        head->next->pre = cur;
        cur->next = head->next;
        cur->pre = head;
        head->next = cur;
    }

    void Print()
    {
        ListNode* cur = head->next;
        while (cur != NULL && cur != tail)
        {
            printf("%d ", cur->key);
            cur = cur->next;
        }
        printf("\n");
    }
};

HashTable* createHashTable(const unsigned int size)
{
   HashTable* table = new HashTable();
   table->size = size;
   table->sizemask = table->size - 1;
   table->buckets = new ListNode*[table->size];
   table->head = new ListNode();
   table->tail = new ListNode();
   table->head->next = table->tail;
   table->tail->pre = table->head;
   return table;
}

int main()
{
    HashTable* table = createHashTable(4);
    table->Set(3, 11);
    table->Set(1, 12);
    table->Set(5, 23);
    table->Set(2, 22);
    table->Set(3, 26);
    table->Get(5);
    table->Print();
    return 0;
}