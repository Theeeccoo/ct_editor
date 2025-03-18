#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"
#include "dynamic_array.h"

darray_tt darray_create(Destructor items_destructor, size_t items_size, bool is_pointer)
{
    assert( items_size > 0 && "ERROR: items_size must be positive" );
    assert( items_destructor != NULL && "Error: items_destructor is NULL" );

    darray_tt da = malloc(sizeof(struct darray));
    da->destructor = items_destructor;
    da->count = 0;
    da->capacity = 0;
    da->items_size = items_size;
    da->is_pointer = is_pointer;

    return (da);
}

void darray_insert(darray_tt da, void* item)
{
    assert( da != NULL && "ERROR: Inserting element in NULL Dynamic array\n");
    if ( da->count >= da->capacity )
    {
        da->capacity = (da->capacity == 0) ? INIT_CAP : da->capacity * 2;
        da->items = realloc(da->items, da->capacity * da->items_size);
        if ( da->items == NULL ) handle_error("ERROR: Unable to realloc.");
    }

    // Converting to char* to make BYTE-wise operation, ensuring correct positioning of elements at darray->items
    void *target = (char *) da->items + (da->count * da->items_size);

    // If working with pointers, memcpy would copy pointer's address, causing SEGFAULT
    if ( da->is_pointer ) *(void**) target = item;
    else memcpy(target, item, da->items_size);
    da->count++;
}

void darray_insert_at(darray_tt da, int position, void* item)
{
    assert( da != NULL && "ERROR: Inserting element in NULL Dynamic array\n");
    assert( ((position > 0) && (position < (int)da->count)) && "ERROR: Inserting element in invalid position\n");

    if ( da->count >= da->capacity )
    {
        da->capacity = (da->capacity == 0) ? INIT_CAP : da->capacity * 2;
        da->items = realloc(da->items, da->capacity * da->items_size);
        if ( da->items == NULL ) handle_error("ERROR: Unable to realloc.");
    }

    void *target = (char *) da->items + (position * da->items_size);
    assert( (memmove((char *) da->items + ((position + 1) * da->items_size), target, ((da->count - position + 1) * da->items_size))) != NULL );

    // If working with pointers, memcpy would copy pointer's address, causing SEGFAULT
    if ( da->is_pointer ) *(void**) target = item;
    else memcpy(target, item, da->items_size);
    da->count++;
}

void darray_remove_at(darray_tt da, int position)
{
    assert( da != NULL && "ERROR: Removing element in NULL Dynamic array\n");
    assert( ((position > 0) && (position < (int)da->count)) && "ERROR: Removing element in invalid position\n");

    if ( da->count < (size_t)(da->capacity/2) )
    {
        da->capacity = (da->capacity < INIT_CAP) ? INIT_CAP : (int)da->capacity / 2;
        da->items = realloc(da->items, da->capacity * da->items_size);
        if ( da->items == NULL ) handle_error("ERROR: Unable to realloc.");
    }

    void *target = (char *) da->items + (position * da->items_size);

    assert( (memmove(target, (char *) da->items + ((position + 1) * da->items_size), ((da->count - position + 1) * da->items_size))) != NULL );
    da->count--;
}

void darray_destroy(darray_tt darray)
{
    if ( darray->destructor != NULL )
    {
        for ( size_t i = 0; i < darray->count; i++ )
        {
            void *item = (char*) darray->items + (i * darray->items_size);
            if (darray->is_pointer) darray->destructor(*(void**)item);
            else darray->destructor(item);
         }
    }

    free(darray->items);
}
