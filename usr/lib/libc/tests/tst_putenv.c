#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main()
{
    printf("Testing putenv...");
    
    putenv("ENV=/usr/bin");
    assert(!strcmp(getenv("ENV"), "/usr/bin"));
    putenv("ENV=/usr/local/bin");
    assert(!strcmp(getenv("ENV"), "/usr/local/bin"));

    printf("passed\n");

    return 0;
}