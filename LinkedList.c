#include "LinkedList.h"

linked_list_t* ll_create(unsigned int data_size)
{
    linked_list_t *list = malloc(sizeof(linked_list_t));
    list->head = NULL;
    
    list->data_size = data_size;
    list->size = 0;

    return list;
}

/*
 * Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e
 * adaugat pe pozitia n a listei reprezentata de pointerul list. Pozitiile din
 * lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla pe
 * pozitia n=0). Daca n >= nr_noduri, noul nod se adauga la finalul listei. Daca
 * n < 0, eroare.
 */

static void ll_add_first_node(linked_list_t *list, const void *new_data) {
    ll_node_t *new_head = malloc(sizeof(ll_node_t));
    
    new_head->data = malloc(list->data_size);
    memcpy(new_head->data, new_data, list->data_size);
    
    new_head->next = list->head;
    list->head = new_head;  

    list->size++;
}

static void ll_add_last_node(linked_list_t *list, const void *new_data) {
    ll_node_t *new_tail = malloc(sizeof(ll_node_t));

    ll_node_t *n = list->head;

    while (n->next) {
        n = n->next;
    }

    n->next = new_tail;
    new_tail->next = NULL;

    new_tail->data = malloc(list->data_size);
    memcpy(new_tail->data, new_data, list->data_size);

    list->size++;
}

void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    if (!list) {
        return ;
    }

    //  add on the first position
    if (n == 0 || list->size == 0) { 
        ll_add_first_node(list, new_data);
        return;
    }

    // add on the last position
    if (list->size <= n) { 
        ll_add_last_node(list, new_data);
        return;
    }

    ll_node_t *curr = list->head;
    unsigned int count = 0;

    while (count < n - 1) {
        curr = curr->next;
        count++;
    }

    ll_node_t *tmp = curr->next;
    ll_node_t *new_node = malloc(sizeof(ll_node_t));
    
    new_node->data = malloc(list->data_size);
    memcpy(new_node->data, new_data, list->data_size);
    curr->next = new_node;
    new_node->next = tmp;

    list->size++;
}



/*
 * Elimina nodul de pe pozitia n din lista al carei pointer este trimis ca
 * parametru. Pozitiile din lista se indexeaza de la 0 (i.e. primul nod din
 * lista se afla pe pozitia n=0). Daca n >= nr_noduri - 1, se elimina nodul de
 * la finalul listei. Daca n < 0, eroare. Functia intoarce un pointer spre acest
 * nod proaspat eliminat din lista. Este responsabilitatea apelantului sa
 * elibereze memoria acestui nod.
 */

static ll_node_t *ll_remove_first_node(linked_list_t* list) {
    ll_node_t *tmp = list->head;
    list->head = tmp->next;

    list->size--;

    return tmp;
}

static ll_node_t *ll_remove_last_node(linked_list_t *list) {
    ll_node_t *curr = list->head;

    while (curr->next->next) {
        curr = curr->next;
    }

    ll_node_t *tmp = curr->next;
    curr->next = NULL;

    list->size--;

    return tmp;
}

ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    if (!list) {
        return NULL;
    }

    if (n == 0) {
        ll_node_t *tmp = ll_remove_first_node(list);
        return tmp;
    }

    if (list->size <= n) {
        ll_node_t *tmp = ll_remove_last_node(list);
        return tmp;
    }

    ll_node_t *curr = list->head;
    unsigned int count = 0;

    while (count < n - 1) {
        curr = curr->next;
        count++;
    }

    ll_node_t *tmp = curr->next;
    curr->next = tmp->next;

    list->size--;

    return tmp;
}

/*
 * Functia intoarce numarul de noduri din lista al carei pointer este trimis ca
 * parametru.
 */
unsigned int ll_get_size(linked_list_t* list)
{
    if (!list) {
        return -1;
    } 
    /* If the list doesn't contain its size
    
    ll_node_t *curr = list->head;
    unsigned int size = 0;
    
    while (curr) {
        curr = curr->next;
        size++;
    }
    return size;
    */

    return list->size;
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
 * sfarsit, elibereaza memoria folosita de structura lista si actualizeaza la
 * NULL valoarea pointerului la care pointeaza argumentul (argumentul este un
 * pointer la un pointer).
 */
void ll_free(linked_list_t** pp_list)
{
    if (!(*pp_list)) {
        return ;
    } 
    
    ll_node_t *curr = (*pp_list)->head;

    while (curr) {
        ll_node_t *tmp = curr;
        
        free(curr->data);
        curr = curr->next;
        
        free(tmp);
    }
    
    free(*pp_list);
    *pp_list = NULL;
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM
 * ca stocheaza int-uri. Functia afiseaza toate valorile int stocate in nodurile
 * din lista inlantuita separate printr-un spatiu.
 */
void ll_print_int(linked_list_t* list)
{
    if (!list) {
        return ;
    }

    ll_node_t *curr = list->head;

    while (curr) {
        printf("%d ", (*(int *)curr->data));
        curr = curr->next;
    }

    printf("\n");
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM
 * ca stocheaza string-uri. Functia afiseaza toate string-urile stocate in
 * nodurile din lista inlantuita, separate printr-un spatiu.
 */
void ll_print_string(linked_list_t* list)
{
    if (!list) {
        return ;
    } 
    ll_node_t *curr = list->head;

    while (curr) {
        printf("%s ", ((char *)curr->data));
        curr = curr->next;
    }

    printf("\n");

    printf("\n");
}
