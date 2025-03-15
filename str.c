#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utils.h"
#include "str.h"

struct string
{
    char*  content;
    size_t content_len;
};

string_tt string_create(const char* str)
{
    assert( str != NULL && "ERROR: Can not create empty String.\n");

    string_tt new_str = malloc(sizeof(struct string));
    if ( new_str == NULL ) handle_error("ERROR: Unnable to malloc string struct");

    new_str->content = malloc(sizeof(char) * strlen(str) + 1);
    if ( new_str->content == NULL ) handle_error("ERROR: Unnable to malloc string");
    // TODO: check if strcpy is executed correctly
    strcpy(new_str->content, str);
    new_str->content_len = strlen(new_str->content);

    return new_str;
}

void string_free(void* str)
{
    assert( str != NULL && "ERROR: Freeing empty String\n");
    string_tt var = (string_tt) str;
    free(var->content);
    var->content = NULL;
    free(var);
    var = NULL;
}

void string_print(const_string_tt str)
{
    assert( str != NULL && "ERROR: Printing empty String\n");
    printf("%s", str->content);
}
