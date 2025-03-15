#ifndef DYNAMIC_ARRAY_H__
#define DYNAMIC_ARRAY_H__
#include <stdbool.h>

#define INIT_CAP 4

typedef struct darray* darray_tt;
typedef const struct darray* const_darray_tt;
typedef void (*Destructor)(void*);

darray_tt darray_create(Destructor, size_t, bool);
void darray_insert(darray_tt, void*);
void darray_destroy(darray_tt);

#endif /** DYNAMIC_ARRAY_H__ */
