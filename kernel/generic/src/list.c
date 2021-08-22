#include <list.h>
#include <types.h>
#include <heap.h>

struct list list_create()
{
    return (struct list) { .head = NULL, .tail = NULL };
}

void list_push_back(struct list* list, void* data)
{
    // Initialize new node
    struct listnode* node = kmalloc(sizeof(struct listnode));
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
}

void* list_pop_front(struct list* list)
{
    if (!list->head) return NULL;

    // Save the old head to return and free
    struct listnode* old = list->head;
    
    // Advance the head pointer
    list->head = list->head->next;
    
    // Try to set the head's previous to NULL, if not than set tail to NULL (no items left in list)
    if (list->head)
        list->head->prev = NULL;
    else
        list->tail = NULL;

    // Clean up and return the data
    void* ret = old->data;
    kfree(old);
    return ret;
}

void list_clear()
{
    // TODO: impl
}