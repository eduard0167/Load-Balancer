#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct ll_node_t ll_node_t;
struct ll_node_t
{
    void* data;
    ll_node_t* next;
};

typedef struct linked_list_t linked_list_t;
struct linked_list_t
{
    ll_node_t* head;
    unsigned int data_size;
    unsigned int size;
};

linked_list_t* ll_create(unsigned int data_size);

void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* data);

ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n);

unsigned int ll_get_size(linked_list_t* list);

void ll_free(linked_list_t** pp_list);

void ll_print_int(linked_list_t* list);

void ll_print_string(linked_list_t* list);

ll_node_t *remove_middle(linked_list_t *list);

#endif /* LINKED_LIST_H_ */
