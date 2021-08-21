#pragma once

struct listnode
{
    struct listnode* next;
    struct listnode* prev;
    void* data;
};

struct list
{
    struct listnode* head;
    struct listnode* tail;
};

#define list_foreach(list) for (struct listnode* node = list->head; node != NULL; node = node->next)

struct list list_create();
void list_push_back(struct list* list, void* data);
void* list_pop_front(struct list* list);
void list_clear();
