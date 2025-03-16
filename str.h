#ifndef STR_H__
#define STR_H__

struct string
{
    char*  content;
    size_t content_len;
};
typedef struct string* string_tt;
typedef const struct string* const_string_tt;

string_tt string_create(const char*);
void string_print(const_string_tt);
void string_free(void*);
void string_append_char_at(string_tt, char, int);
void string_delete_char_at(string_tt, int);

#endif /* STR_H__ */
