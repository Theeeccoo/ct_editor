#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

void handle_error(const char* msg)
{
    fprintf(stdout, "%s: ", msg);
    fprintf(stdout, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
}
