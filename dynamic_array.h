#ifndef DYNAMIC_ARRAY_H__
#define DYNAMIC_ARRAY_H__
#include <stdbool.h>

#define INIT_CAP 4

typedef void (*Destructor)(void*);
typedef struct darray
{
    void** items;          /**< Dynamic array items.                       */
    Destructor destructor; /**< Dynamic array items' destructor.           */
    size_t count;          /**< Current number of items.                   */
    size_t capacity;       /**< Dynamic array current capacity.            */
    size_t items_size;     /**< Size of each element in bytes.             */
    bool is_pointer;       /**< If Dynamic array is working with pointers. */
}darray;

typedef struct darray* darray_tt;
typedef const struct darray* const_darray_tt;

darray_tt darray_create(Destructor, size_t, bool);
void darray_insert(darray_tt, void*);
void darray_insert_at(darray_tt, int, void*);
void darray_destroy(darray_tt);

#endif /** DYNAMIC_ARRAY_H__ */
