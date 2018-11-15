#ifndef __LINKLIST_H__
#define __LINKLIST_H__

// 链表节点
typedef struct ListStruct
{
    char name[256];
    char value[2048];
    struct ListStruct *next;
} ListStruct;
// 链表表头
typedef struct ListHead
{
    ListStruct *first;
    int size;
} ListHead;
// 创建一个链表
ListHead* link_create();
// 获取链表长度
int link_get_size(ListHead* link_head);
// 通过name获取value
ListStruct* link_get(ListHead* link_head, char *name);
// 通过name设置value的值
int link_set(ListHead* link_head, char *name, char *value);
// 新增一个节点
void link_add(ListHead* link_head, char *name, char *value);
// 删除一个节点
int link_del(ListHead* link_head, char *name);
// 输出链表
void link_print(ListHead* link_head);
// 输出单个节点
void link_print_node(ListStruct *node);
// 清空链表
void link_free(ListHead* link_head);
// 通过index获取value，index从0开始
ListStruct* link_get_index(ListHead* link_head, int index);
#endif