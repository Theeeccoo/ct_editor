#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utils.h"
#include "str.h"

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

string_tt string_content_from(string_tt str, int position)
{
    assert( str != NULL && "ERROR: Returning content from empty String\n");
    assert( ((position >= 0) && (position < (int) str->content_len)) && "ERROR: Invalid position to access.\n");

    size_t new_length = str->content_len - position;
    char* auxiliary = malloc(new_length + 1);
    if ( auxiliary == NULL ) handle_error("ERROR: Unnable to malloc auxiliary string\n");

    memcpy(auxiliary, &str->content[position], (new_length * sizeof(char)));
    auxiliary[new_length] = '\0';

    char* new_str = realloc(str->content, position + 2);
    if ( new_str == NULL ) handle_error("ERROR: Unnable to realloc string\n");
    str->content = new_str;
    str->content[position] = '\n';
    str->content[position + 1] = '\0';
    str->content_len = position + 2;

    string_tt return_string = string_create(auxiliary);
    free(auxiliary);
    return return_string;
}

void string_print(const_string_tt str)
{
    assert( str != NULL && "ERROR: Printing empty String\n");
    printf("%s", str->content);
}
void string_append_char_at(string_tt str, char new_c, int position)
{
    assert( str != NULL && "ERROR: Appending into empty String\n");
    assert( ((position >= 0) && (position < (int) str->content_len)) && "ERROR: Invalid position to append.\n");

    char* new_str = realloc(str->content, str->content_len + 2);
    if ( new_str == NULL ) handle_error("ERROR: Unnable to realloc string\n");
    str->content = new_str;
    assert( (memmove(str->content + position + 1, str->content + position, ((str->content_len - position + 1) * sizeof(char)))) != NULL );
    str->content[position] = new_c;
    str->content_len++;
}

void string_delete_char_at(string_tt str, int position)
{
    assert( str != NULL && "ERROR: Appending into empty String\n");
    assert( ((position >= 0) && (position < (int) str->content_len)) && "ERROR: Invalid position to append.\n");

    char* new_str = realloc(str->content, str->content_len - 1);
    if ( new_str == NULL ) handle_error("ERROR: Unnable to realloc string\n");
    str->content = new_str;
    assert( (memmove(str->content + position, str->content + position + 1, ((str->content_len - position + 1) * sizeof(char)))) != NULL );
    str->content_len--;
}
