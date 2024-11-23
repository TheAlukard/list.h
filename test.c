#include "list.h"

void add5(void *l)
{
    list_of(int) *list = l;

    list_push(list, 1);
    list_push(list, 2);
    list_push(list, 3);
    list_push(list, 4);
    list_push(list, 5);
}

int main(void)
{
    list_of(int) list = {0};

    add5(&list);

    list_print(&list, "%d");

    printf("popped: %d\n", list_pop(&list));
    list_print(&list, "%d");

    return 0;
}
