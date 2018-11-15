#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linklist.h"

// 创建一个链表
ListHead* link_create()
{
    ListHead *head; // 定义头节点
    head = (ListHead*)malloc(sizeof(ListHead)); // 分配地址
    head->first = NULL;
    head->size = 0;
    return head;
}
// 获取链表长度
int link_get_size(ListHead* link_head)
{
    return link_head->size;
}
// 通过name获取value
ListStruct* link_get(ListHead* link_head, char *name)
{
    ListStruct *link_node;
    for (link_node = link_head->first; link_node != NULL; link_node = link_node->next)
    {
        if (strcmp(link_node->name, name) == 0)
        {
            return link_node;
        }
    }
    return NULL;
}
// 通过name设置value的值
int link_set(ListHead* link_head, char *name, char *value)
{
    ListStruct *link_node = link_get(link_head, name);
    if (link_node == NULL)
    {
        return 0;
    }
    strcpy(link_node->value, value);
    return 1;
}
// 新增一个节点
void link_add(ListHead* link_head, char *name, char *value)
{
    ListStruct *node = (ListStruct*)malloc(sizeof(ListStruct));
    strcpy(node->name, name);
    strcpy(node->value, value);
    if (link_head->first == NULL)
    {
        node->next = NULL;
    }
    else
    {
        node->next = link_head->first;
    }
    link_head->first = node;
    link_head->size++;
}
// 删除一个节点
int link_del(ListHead* link_head, char *name)
{
    // 没有节点的情况
    if (link_head->size == 0)
    {
        return 0;
    }

    ListStruct *pre = NULL;
    ListStruct *tag;
    for (tag = link_head->first; tag != NULL; pre = tag, tag = tag->next)
    {
        if (strcmp(tag->name, name) == 0)
        {
            if (pre == NULL)
            {
                link_head->first = tag->next;
            }
            else
            {
                pre->next = tag->next;
            }
            free(tag);
            link_head->size--;
            return 1;
        }
    }
    return 0;
}
// 输出链表
void link_print(ListHead* link_head)
{   
    if (link_head == NULL)
    {
        printf("empty\n");
        return;
    }
    ListStruct *node = link_head->first;
    printf("len=%d\n", link_head->size);
    while (node != NULL)
    {
        printf("node=%x\tname=%s\tvalue=%s\tnext=%x\n", node, node->name, node->value, node->next);
        node = node->next;
    }
}
// 输出单个节点
void link_print_node(ListStruct *node)
{
    printf("node=%x\tname=%s\tvalue=%s\tnext=%x\n", node, node->name, node->value, node->next);
}
// 清空链表
void link_free(ListHead* link_head)
{
    ListStruct *node, *next;
    node = link_head->first;
    if (node != NULL) 
    {
        do {
            next = node->next;
            free(node);
            node = next;
        } while (next != NULL);
    }
    free(link_head);
}
// 通过index获取value，index从0开始
ListStruct* link_get_index(ListHead* link_head, int index)
{
    int link_len = link_get_size(link_head) - 1;
    ListStruct *tag;
    int real_index = link_len - index;
    int count = 0;
    tag = link_head->first;
    do
    {
        if (count == real_index)
        {
            return tag;
        }
        tag = tag->next;
        count++;
    } while (count < link_len);

    return NULL;
}