#include <micro/list.h>
#include <micro/types.h>
#include <micro/heap.h>

struct list list_create()
{
    return (struct list) { .head = NULL, .tail = NULL, .size = 0 };
}

void list_push_back(struct list* list, void* data)
{
    // Initialize new node
    struct lnode* node = kmalloc(sizeof(struct lnode));
    node->next = NULL;
    node->prev = list->tail;
    node->data = data;

    // Try to adjust the tail, if not than set head (no tail means no head)
    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;

    // Set the new tail
    list->tail = node;

    list->size++;
}

void* list_dequeue(struct list* list)
{
    if (!list->head) return NULL;

    // Save the old head to return and free
    struct lnode* old = list->head;
    
    // Advance the head pointer
    list->head = list->head->next;
    
    // Try to set the head's previous to NULL, if not than set tail to NULL (no items left in list)
    if (list->head)
        list->head->prev = NULL;
    else
        list->tail = NULL;

    list->size--;

    // Clean up and return the data
    void* ret = old->data;
    kfree(old);
    return ret;
}

void* list_pop_back(struct list* self)
{
    if (!self->tail) return NULL;

    // Save the old tail to return and free
    struct lnode* old = self->tail;
    
    // Decrement the tail pointer
    self->tail = self->tail->prev;
    
    // Try to set the tail's next to NULL, if not than set head to NULL (no items left in list)
    if (self->tail)
        self->tail->next = NULL;
    else
        self->head = NULL;

    self->size--;

    // Clean up and return the data
    void* ret = old->data;
    kfree(old);
    return ret;
}

void list_clear(struct list* self)
{
    while (self->size) list_dequeue(self);
}

void* list_get(struct list* self, size_t i)
{
    struct lnode* node = self->head;
    while (i && i--) node = node->next; // i=0 will overflow if not checked
    return node->data;
}