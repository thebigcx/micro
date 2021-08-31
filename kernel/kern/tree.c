#include <tree.h>
#include <types.h>
#include <heap.h>

struct tree tree_create()
{
    return (struct tree) { .children = list_create(), .data = NULL };
}

void tree_push_back(struct tree* self, void* data)
{
    struct tree* node = kmalloc(sizeof(struct tree));
    node->children = list_create();
    node->data = data;

    list_push_back(&self->children, node);
}