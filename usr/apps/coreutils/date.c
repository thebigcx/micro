#include <time.h>

int main(int argc, char** argv)
{
    time_t curr = time(NULL);

    printf("%s", ctime(&curr));

    return 0;
}