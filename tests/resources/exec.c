#include "burn_cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <unistd.h>

int main(int argc, char *argv[])
{
    assert(argc == 1 + 1);
    const int level = atoi(argv[1]) + 1;
    fprintf(stderr, "%d\n", level);
    fflush(stderr);
    burn_cpu(1000 * 1000);
    char lvl[10];
    sprintf(lvl, "%d", level);
    if (level)
        execl(argv[0], argv[0], lvl, (char *)0);
    return 0;
}
