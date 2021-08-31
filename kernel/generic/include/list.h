#pragma once

#include <types.h>

struct lnode
{
    struct lnode* next;
    struct lnode* prev;
    void* data;
};

struct list
{
    struct lnode* head;
    struct lnode* tail;
    size_t size;
};

#define LIST_FOREACH(list) for (struct lnode* node = (list)->head; node != NULL; node = node->next)

struct list list_create();
void list_push_back(struct list* self, void* data);
void* list_pop_front(struct list* self);
void list_clear();