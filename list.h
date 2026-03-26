#pragma once

#ifndef LIST__H__
#define LIST__H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
    #define ALWAYS_INLINE __attribute__((always_inline)) static inline
#else
    #define ALWAYS_INLINE static inline
#endif

#define C23 202311

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#ifndef DEFAULT_LIST_CAP
    #define DEFAULT_LIST_CAP 64
#endif

typedef struct {
    void *items;
    size_t count;
    size_t capacity;
} LIST_BLUEPRINT;

#define LIST_STRUCT(name, type) \
    typedef struct {            \
        type *items;            \
        size_t count;           \
        size_t capacity;        \
    } name

#define list_of(type) list__##type

#define LIST_DEFINE(type) \
    typedef struct {      \
        type *items;      \
        size_t count;     \
        size_t capacity;  \
    } list_of(type)

#define list_alloc(list, cap)                                              \
    do {                                                                   \
        (list)->capacity = (cap);                                          \
        (list)->count = 0;                                                 \
        (list)->items = malloc((list)->capacity * sizeof(*(list)->items)); \
    } while (0)

#define list_free(list)       \
    do {                      \
        free((list)->items);  \
        (list)->items = NULL; \
        (list)->count = 0;    \
        (list)->capacity = 0; \
    } while (0)

#define list_accomodate(list)                                                                   \
    do {                                                                                        \
        if ((list)->items == NULL || (list)->count >= (list)->capacity) {                       \
            (list)->capacity = (list)->capacity <= 0 ? DEFAULT_LIST_CAP : (list)->capacity * 2; \
            (list)->items = realloc((list)->items, (list)->capacity * sizeof(*(list)->items));  \
        }                                                                                       \
    } while (0)

#define list_push(list, item)                \
    do {                                     \
        list_accomodate(list);               \
        (list)->items[(list)->count] = item; \
        (list)->count += 1;                  \
    } while (0)

#define list_insert(list, item, index)                                                                                       \
    do {                                                                                                                     \
        if ((index) < 0) break;                                                                                              \
        if ((index) >= (list)->count) {                                                                                      \
            list_push(list, item);                                                                                           \
        }                                                                                                                    \
        else {                                                                                                               \
            list_accomodate(list);                                                                                           \
            memmove(&(list)->items[(index) + 1], &(list)->items[index], sizeof(*(list)->items) * ((list)->count - (index))); \
            (list)->items[index] = item;                                                                                     \
            (list)->count += 1;                                                                                              \
        }                                                                                                                    \
    } while (0)

#define list_from_arr(list, arr, arr_count)                               \
    do {                                                                  \
        if (!(arr) || (arr_count) <= 0) break;                            \
        list_alloc(list, arr_count);                                      \
        memcpy((list)->items, arr, (arr_count) * sizeof(*(list)->items)); \
        (list)->count += (arr_count);                                     \
    } while(0)

#define ptr_offset(ptr, idx, typesize) ((uint8_t*)(ptr) + ((idx) * (typesize)))

ALWAYS_INLINE void* LIST_GET_POPPED(void *list_ptr, size_t type_size) 
{
    void *popped = NULL;
    LIST_BLUEPRINT *list = (LIST_BLUEPRINT*)list_ptr;

    if (list->count == 0) return popped;

    if (list->count < list->capacity / 3) {
        list->capacity /= 2;
        list->items = realloc(list->items, list->capacity * type_size);
    }

    list->count -= 1;
    popped = ptr_offset(list->items, list->count, type_size);

    return popped;
}

ALWAYS_INLINE void* LIST_GET_REMOVED(void* list_ptr, size_t index, size_t type_size, bool ordered)
{
    void *removed = NULL;

    LIST_BLUEPRINT *list = (LIST_BLUEPRINT*)list_ptr;

    if (list->count == 0 || index >= list->count) return removed;

    if (list->count < list->capacity / 3) {
        list->capacity /= 2;
        list->items = realloc(list->items, list->capacity * type_size);
    }

    uint8_t temp[type_size];
    list->count -= 1;
    removed = ptr_offset(list->items, index, type_size);
    void *last_addr = ptr_offset(list->items, list->count, type_size);
    memmove(temp, removed, type_size);

    if (ordered) {
        if (index + 1 < list->count)
            memmove(removed, (uint8_t*)removed + type_size, type_size * (list->count - (index + 1)));
    }
    else {
        memmove(removed, last_addr, type_size);
    }
    memmove(last_addr, temp, type_size);

    return last_addr;
}

#if __STDC_VERSION__ >= C23
    #define list_pop(list) (*(typeof(*(list)->items)*)LIST_GET_POPPED(list, sizeof(*(list)->items)))
    #define list_remove(list, index) (*(typeof(*(list)->items)*)LIST_GET_REMOVED(list, index, sizeof(*(list)->items), false))
    #define list_remove_order(list, index) (*(typeof(*(list)->items)*)LIST_GET_REMOVED(list, index, sizeof(*(list)->items), true))
#elif defined(__GNUC__) || defined(__clang__)
    #define list_pop(list) (*(__typeof__(*(list)->items)*)LIST_GET_POPPED(list, sizeof(*(list)->items)))
    #define list_remove(list, index) (*(__typeof__(*(list)->items)*)LIST_GET_REMOVED(list, index, sizeof(*(list)->items), false))
    #define list_remove_order(list, index) (*(__typeof__(*(list)->items)*)LIST_GET_REMOVED(list, index, sizeof(*(list)->items), true))
#else
    #define list_pop(list, type) (*(type(*(list)->items)*)LIST_GET_POPPED(list, sizeof(*(list)->items)))
    #define list_remove(list, index, type) (*(type(*(list)->items)*)LIST_GET_REMOVED(list, index, sizeof(*(list)->items), false))
    #define list_remove_order(list, index) (*(type(*(list)->items)*)LIST_GET_REMOVED(list, index, sizeof(*(list)->items), true))
#endif

ALWAYS_INLINE bool LIST_CONTAINS_ITEM(void *items, size_t count, size_t item_size, void *item)
{
    for (size_t i = 0; i < count; i++) {
        uint8_t *cur = ((uint8_t*)items) + (i * item_size);
        if (memcmp(cur, item, item_size) == 0) return true;
    }

    return false;
}

#define list_contains(list, item) (LIST_CONTAINS_ITEM((list)->items, (list)->count, sizeof(*(list)->items), &(item)))

#define list_copy(dest, src, start, num)            \
    do {                                            \
        if ((start) < 0) {                          \
            break;                                  \
        }                                           \
        size_t i = (start);                         \
        size_t j = 0;                               \
        while (j < (num) && i < (src)->count) {     \
            if (j >= (dest)->count) {               \
                list_push(dest, (src)->items[i]);   \
            }                                       \
            else {                                  \
                (dest)->items[j] = (src)->items[i]; \
            }                                       \
            i += 1;                                 \
            j += 1;                                 \
        }                                           \
    } while (0)

#define list_transfer(dest, src)            \
    do {                                    \
        free((dest)->items);                \
        (dest)->items = (src)->items;       \
        (dest)->capacity = (src)->capacity; \
        (dest)->count = (src)->count;       \
    } while (0)

#define list_print(list, format)                     \
    do {                                             \
        printf("[");                                 \
        for (size_t i = 0; i < (list)->count; i++) { \
            printf(format, (list)->items[i]);        \
            if (i < (list)->count - 1) {             \
                printf(", ");                        \
            }                                        \
        }                                            \
        printf("]\n");                               \
    } while (0)

#define list_clear(list) ((list)->count = 0)

#undef ptr_offset
#undef ALWAYS_INLINE
#undef C23

#endif // LIST__H__
