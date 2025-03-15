#ifndef STR_H__
#define STR_H__

typedef struct string* string_tt;
typedef const struct string* const_string_tt;

string_tt string_create(const char*);
void string_print(const_string_tt);
void string_free(void*);

#endif /* STR_H__ */
