#pragma once

#ifndef LIST__H__
#define LIST__H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define INLINE __attribute__((always_inline)) static inline

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#ifndef DEFAULT_LIST_CAP
  #define DEFAULT_LIST_CAP 64
#endif

#define list_of(type) \
  struct {\
    type *items;\
    size_t count;\
    size_t capacity;\
  }

#define LIST_DEF(name, type) \
  typedef struct {\
    type *items;\
    size_t count;\
    size_t capacity;\
  } name

#define list_new(type) ((type){.items = NULL, .count = 0, .capacity = 0})

#define list_alloc(list)                                                    \
  do {                                                                      \
    (list)->capacity = DEFAULT_LIST_CAP;                                    \
    (list)->count = 0;                                                      \
    (list)->items = malloc((list)->capacity * sizeof(*(list)->items));      \
  } while (0)

#define list_free(list)                                                     \
  do {                                                                      \
    free((list)->items);                                                    \
    (list)->items = NULL;                                                   \
    (list)->count = 0;                                                      \
    (list)->capacity = 0;                                                   \
  } while (0)

#define list_accomodate(list)                                                             \
  do {                                                                                    \
    if ((list)->count >= (list)->capacity) {                                              \
      (list)->capacity = (list)->capacity <= 0 ? DEFAULT_LIST_CAP : (list)->capacity * 2; \
      (list)->items = realloc((list)->items, (list)->capacity * sizeof(*(list)->items));  \
    }                                                                                     \
  } while (0)

#define list_push(list, item)                                                            \
  do {                                                                                   \
    list_accomodate(list);                                                               \
    (list)->items[(list)->count] = item;                                                 \
    (list)->count += 1;                                                                  \
  } while (0)

#define list_insert(list, item, index)                                                      \
  do {                                                                                      \
    if ((index) < 0) break;                                                                 \
    if ((index) >= (list)->count) {                                                         \
      list_push(list, item);                                                                \
    }                                                                                       \
    else {                                                                                  \
      list_accomodate(list);                                                                \
      memmove(&(list)->items[(index) + 1], &(list)->items[index], (list)->count - (index)); \
      (list)->items[index] = item;                                                          \
      (list)->count += 1;                                                                   \
    }                                                                                       \
  } while (0)

INLINE void* LIST_GET_POPPED(void* *list_items, size_t type_size, size_t *list_count, size_t *list_cap) 
{
    void *popped = NULL; 

    if (*list_count == 0) return popped;

    if (*list_count < (*list_cap) / 3) {
        *list_cap /= 2;
        *list_items = realloc(*list_items, (*list_cap) * type_size);
    }

    *list_count -= 1;

    popped = (uint8_t*)(*list_items) + ((*list_count) * type_size);

    return popped;
}

#define list_pop(list) (*(typeof(*(list)->items)*)LIST_GET_POPPED((void*)(&(list)->items), sizeof(*(list)->items), &(list)->count, &(list)->capacity))

#define list_remove(list, index)                                                                  \
  do {                                                                                            \
    if ((list)->count <= 0 || index < 0) break;                                                   \
    if ((index) < (list)->count - 1) {                                                            \
      memmove(&(list)->items[index], &(list)->items[(index) + 1], (list)->count - ((index) + 1)); \
    }                                                                                             \
    (list)->count -= 1;                                                                           \
    if ((list)->count < (list)->capacity / 3) {                                                   \
      (list)->capacity /= 2;                                                                      \
      (list)->items = realloc((list)->items, (list)->capacity * sizeof(*(list)->items));          \
    }                                                                                             \
  } while (0)

INLINE bool LIST_CONTAINS_ITEM(void *items, size_t count, size_t item_size, void *item)
{
    for (size_t i = 0; i < count; i++) {
        uint8_t *cur = ((uint8_t*)items) + (i * item_size);
        if (memcmp(cur, item, item_size) == 0) return true;
    }

    return false;
}

#define list_contains(list, item) (LIST_CONTAINS_ITEM((list)->items, (list)->count, sizeof(*(list)->items), &(item)))

#define list_copy(dest, src, start, count)                                   \
  do {                                                                       \
    if ((start) < 0) {                                                       \
      break;                                                                 \
    }                                                                        \
    size_t i = (start);                                                      \
    size_t j = 0;                                                            \
    while (i < (count) && i < (src)->count && j < (dest)->count) {           \
      (dest)->items[j] = (src)->items[i];                                    \
      i += 1;                                                                \
      j += 1;                                                                \
    }                                                                        \
  } while (0)

#define list_transfer(dest, src)                                             \
  do {                                                                       \
    if ((dest)->items != NULL) {                                             \
      free((dest)->items);                                                   \
      (dest)->items = NULL;                                                  \
    }                                                                        \
    (dest)->items = (src)->items;                                            \
    (dest)->capacity = (src)->capacity;                                      \
    (dest)->count = (src)->count;                                            \
  } while (0)

#define list_print(list, format)                                             \
  do {                                                                       \
    printf("[");                                                             \
    for (size_t i = 0; i < (list)->count; i++) {                             \
      printf(format, (list)->items[i]);                                      \
      if (i < (list)->count - 1) {                                           \
        printf(", ");                                                        \
      }                                                                      \
    }                                                                        \
    printf("]\n");                                                           \
  } while (0)

#define list_clear(list) ((list)->count = 0)

#endif // LIST__H__
