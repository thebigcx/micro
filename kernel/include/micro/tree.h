#pragma once

#include <micro/list.h>

struct tree
{
    struct list children;
    void* data;
};

struct tree tree_create();
void tree_push_back(struct tree* self, void* data);