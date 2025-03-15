#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "str.h"

void handle_error(const char* msg)
{
    fprintf(stdout, "%s: ", msg);
    fprintf(stdout, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

struct string
{
    char*  content;
    size_t content_len;
};

string_tt string_create(const char* str)
{
    assert( str != NULL && "ERROR: Can not create empty String.\n");

    string_tt new_str = malloc(sizeof(struct string));
    if ( new_str == NULL ) handle_error("ERROR - Unnable to malloc string");

    // TODO: check if STRfunctions are executed correctly

    // Ensuring NULL termination
    char aux_char[strlen(str) + 1];
    strcpy(aux_char, str);
    strcat(aux_char, "\0");

    new_str->content_len = strlen(aux_char);
    new_str->content = malloc(sizeof(char) * new_str->content_len);
    strcpy(new_str->content, aux_char);

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
    printf("%s\n", str->content);
}
