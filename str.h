#ifndef STR_H__
#define STR_H__

#include <stdbool.h>

struct string
{
    char*  content;
    size_t content_len;
};
typedef struct string* string_tt;
typedef const struct string* const_string_tt;

string_tt string_create(const char*);
void string_print(const_string_tt);
string_tt string_content_from(string_tt, int);
void string_append_string(string_tt, char*);
void string_append_char_at(string_tt, char, int);
void string_delete_char_at(string_tt, int);
void string_free(void*);

#endif /* STR_H__ */
