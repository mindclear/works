#include <stdio.h>

struct ListNode
{
    int val;
    bool color; //false-黑色 true-红色
    struct ListNode* left;
    struct ListNode* right;
    struct ListNode* parent;

    ListNode():
        val(0), color(true), left(NULL), right(NULL), parent(NULL)
    {}

    ListNode(int _val):
        val(_val), color(true), left(NULL), right(NULL), parent(NULL)
    {}
};

void LeftRotate(ListNode** head, ListNode* cur)
{
    ListNode* right_child = cur->right;
    cur->right = right_child->left;
    if (right_child->left != NULL)
        right_child->left->parent = cur;
    right_child->left = cur;
    right_child->parent = cur->parent;
    cur->parent = right_child;
    if (right_child->parent != NULL)
    {
        if (right_child->parent->left == cur)
            right_child->parent->left = right_child;
        else
            right_child->parent->right = right_child;
    }
    else
    {
        *head = right_child;
    }
}

void RightRotate(ListNode** head, ListNode* cur)
{
    ListNode* left_child = cur->left;
    cur->left = left_child->right;
    if (left_child->right != NULL)
        left_child->right->parent = cur;
    left_child->right = cur;
    left_child->parent = cur->parent;
    cur->parent = left_child;
    if (left_child->parent != NULL)
    {
        if (left_child->parent->left == cur)
            left_child->parent->left = left_child;
        else
            left_child->parent->right = left_child;
    }
    else
    {
        *head = left_child;
    }
}

void RBInsertFIxup(ListNode** head, ListNode* node)
{
    while (node->parent != NULL && node->parent->color) //父结点也是红色
    {
        ListNode* node_parent = node->parent;
        if (node_parent->parent->left == node_parent) //父结点是左结点
        {
            ListNode* node_uncle = node_parent->parent->right;
            if (node_uncle != NULL && node_uncle->color) //父结点的兄弟结点也是红色的
            {
                node_parent->color = false;
                node_uncle->color = false;
                node_parent->parent->color = true;
                node = node_parent->parent;
                continue;
            }
            if (node == node_parent->right) //当前结点是左结点
            {
                LeftRotate(head, node_parent);
                node = node_parent;
                node_parent = node->parent; //旋转跟换了父子结点的顺序
            }
            RightRotate(head, node_parent->parent);
            node_parent->color = false;
            node_parent->right->color = true;
            return;
        }
        else
        {
            ListNode* node_uncle = node_parent->parent->left;
            if (node_uncle != NULL && node_uncle->color) //父结点的兄弟结点也是红色的
            {
                node_parent->color = false;
                node_uncle->color = false;
                node_parent->parent->color = true;
                node = node_parent->parent;
                continue;
            }
            if (node == node_parent->left)
            {
                RightRotate(head, node_parent);
                node = node_parent;
                node_parent = node->parent;
            }
            LeftRotate(head, node_parent->parent);
            node_parent->color = false;
            node_parent->left->color = true;
            return;
        }
        
    }
    node->color = false;
}

void RBInsert(ListNode** head, ListNode* node)
{
    if (*head == NULL)
    {
        *head = node;
        node->color = false;
        return;
    }

    ListNode* pre = NULL;
    ListNode* cur = *head;
    while (cur != NULL)
    {
        pre = cur;
        if (node->val < cur->val)
            cur = cur->left;
        else
            cur = cur->right;
    }

    node->parent = pre;
    if (node->val < pre->val)
        pre->left = node;
    else
        pre->right = node;
    if (pre->color)
        RBInsertFIxup(head, node);
}

void Print(ListNode* head)
{
    if (head == NULL)
        return;

    Print(head->left);
    printf("%d ", head->val);
    Print(head->right);
}

int main()
{
    ListNode* head = NULL;
    RBInsert(&head, new ListNode(20));
    RBInsert(&head, new ListNode(10));
    RBInsert(&head, new ListNode(6));
    RBInsert(&head, new ListNode(12));
    RBInsert(&head, new ListNode(14));
    // RBInsert(&head, new ListNode(13));
    Print(head);
}