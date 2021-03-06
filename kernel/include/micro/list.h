#pragma once

#include <micro/types.h>

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
void list_free(struct list* list);
void* list_enqueue(struct list* self, void* data);
void* list_dequeue(struct list* self);
void* list_pop_back(struct list* self);
void list_clear(struct list* self);
void* list_get(struct list* self, size_t i);
void* list_set(struct list* self, size_t i, void* data);
void* list_remove(struct list* self, size_t i);
void* list_back(struct list* self);
void* list_insert_after(struct list* self, unsigned int i, void* data);
void* list_insert_before(struct list* self, unsigned int i, void* data);
