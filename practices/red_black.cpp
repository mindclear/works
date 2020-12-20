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

ListNode* TreeSuccessor(ListNode* cur)
{
    if (cur == NULL)
        return NULL;

    if (cur->right != NULL) //包含右子树
    {
        ListNode* node = cur->right;
        while (node->left != NULL)
            node = node->left;
        return node;
    }

    while (cur->parent != NULL && cur->parent->right == cur)
        cur = cur->parent;
    return cur->parent;
}

void RBInsertFixup(ListNode** head, ListNode* node)
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
            if (node == node_parent->right) //当前结点是右结点
            {
                LeftRotate(head, node_parent);
                node = node_parent;
                node_parent = node->parent; //旋转更换了父子结点的顺序
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
            if (node == node_parent->left) //当前结点是左结点
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
    if (head == NULL || *head == NULL)
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
        RBInsertFixup(head, node);
}

void RBDeleteFixup(ListNode** head, ListNode* node)
{
    if (NULL == node)
        return;
    
    while (node != *head && !node->color) //当前结点是黑色的
    {
        ListNode* node_parent = node->parent;
        if (node_parent->left == node) //当前结点是左结点
        {
            ListNode* brother = node_parent->right;
            if (brother == NULL) //父结点的右结点为空
            {
                node = node_parent;
                continue;
            }
            else
            {
                if (brother->color) //兄弟结点是红色
                {
                    LeftRotate(head, node_parent);
                    brother->color = false;
                    node_parent->color = true;
                    continue;
                }
                else //兄弟结点是黑色
                {
                    ListNode* brother_left = brother->left;
                    ListNode* brother_right = brother->right;
                    if ((brother_left == NULL || !brother->left->color) && (brother_right == NULL || !brother_right->color)) //左右结点都是黑色
                    {
                        brother->color = true;
                        node = node_parent;
                        continue;
                    }
                    if (brother_right == NULL || !brother_right->color) //左结点是红色，右结点是黑色
                    {
                        RightRotate(head, brother);
                        brother->color = true;
                        brother_left->color = false;
                        brother = brother_left; //旋转更换了node的兄弟结点
                    }
                    LeftRotate(head, node_parent);
                    brother->color = node_parent->color;
                    node_parent->color = false;
                    if (brother_right != NULL) brother_right->color = false;
                    break;
                }
            }
        }
        else //当前结点是右结点
        {
            //TODO
        }
        
    }
    node->color = false;
}

void RBDelete(ListNode** head, ListNode* node)
{
    if (head == NULL || *head == NULL)
        return;

    ListNode* cur = node;
    if (cur->left != NULL & cur->right != NULL)
        cur = TreeSuccessor(cur);

    ListNode* child = (cur->left != NULL) ? cur->left : cur->right; //被删除结点只会有一个子结点
    if (child != NULL)
        child->parent = cur->parent;
    if (cur->parent != NULL)
    {
        if (cur == cur->parent->left)
            cur->parent->left = child;
        else
            cur->parent->right = child;
    }
    else
    {
        *head = child;
    }
    if (cur != node)
    {
        //简单替换信息
        node->val = cur->val;
    }

    if (!cur->color) //被删除结点是黑色的
    {
        if (child != NULL)
            RBDeleteFixup(head, child);
        else if (cur->parent != NULL)
            RBDeleteFixup(head, cur->parent);
    }
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