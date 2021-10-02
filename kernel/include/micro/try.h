#pragma once

#define TRY(fn)                 \
    {                           \
        int e = fn;             \
        if (e < 0) return e;    \
    }

// Error-check function with post call
#define TRY2(fn, post)          \
    {                           \
        int e = fn;             \
        post;                   \
        if (e < 0) return e;    \
    }

// Error-check function with an on-fail
#define TRY3(fn, onfail)        \
    {                           \
        int e = fn;             \
        if (e < 0)              \
        {                       \
            onfail;             \
            return e;           \
        }                       \
    }