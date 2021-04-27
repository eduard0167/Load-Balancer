/* Copyright 2021 Mitroi Eduard Ionut */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

/*
 *  Functia initilizeaza memoria pentru load balancer si paramatrii sai:
 *  array-ul de servere, hash ring (array care retine etichetele replicilor)
 *  si initializeaza cu 0 numarul serverelor (va creste pentru fiecare server
 *  cu 3, retinand in realitate numarul de elemente din hash ring/de replici).
*/
load_balancer_t* init_load_balancer() {
	load_balancer_t *main_lb = malloc(sizeof(*main_lb));
    DIE(!main_lb, "Error in malloc() for load balancer\n");

    main_lb->servers = calloc(MAX_SERVERS, sizeof(server_memory*));
    DIE(!main_lb->servers, "Error in calloc() for array of servers\n");

    main_lb->hash_ring = calloc(MAX_HASH, sizeof(unsigned int));
    DIE(!main_lb, "Error in calloc() for hash ring\n");

    main_lb->num_servers = 0;

    return main_lb;
}

/*
 *  Stocheaza cheia alaturi de valoarea corespunzatoare in primul serverul al
 *  carei replici se gaseste pe hashring cu hash mai mare decat cel al cheii.
*/
void loader_store(load_balancer_t* main, char* key, char* value,
                    int* server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return;
    }

    unsigned int key_hash = hash_function_key(key);
    int n = main->num_servers;
    int pos = binary_search(main->hash_ring, 0, n - 1, key_hash);

    if (pos == main->num_servers) {
        pos = 0;
    }

    *server_id = main->hash_ring[pos] % MAX_SERVERS;
    server_store(main->servers[*server_id], key, value);
}

/*
 * Returneaza valoarea elementului care corespunde unei chei date in mod optim,
 * gasind serverul pe care acesta se afla prin cautare binara in hashring.
*/
char* loader_retrieve(load_balancer_t* main, char* key, int* server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return NULL;
    }

    unsigned int key_hash = hash_function_key(key);
    int n = main->num_servers;
    int pos = binary_search(main->hash_ring, 0, n - 1, key_hash);

    if (pos == main->num_servers) {
        pos = 0;
    }

    *server_id = main->hash_ring[pos] % MAX_SERVERS;
    char *value = server_retrieve(main->servers[*server_id], key);

	return value;
}

/*
 *  Functia se ocupa de realocarea elementelor dintr-un server src intr-un
 *  server dest. Pentru a realiza acest lucru, se parcurg toate elemenele
 *  din serverul sursa si in functie hash-urile cheilor, se face realocarea
 *  pe serverul destinatie, conditia generala fiind ca hash-ul unei chei sa se
 *  afle pe hash ring intre serverul prev si dest.
*/
static void reallocate(load_balancer_t *main, int pos, unsigned int src_tag,
                            unsigned int dest_tag, unsigned int prev_tag,
                            unsigned int server_hash) {
    unsigned int src_id = src_tag % MAX_SERVERS;
    unsigned int dest_id = dest_tag % MAX_SERVERS;
    unsigned int prev_hash =  hash_function_servers(&prev_tag);
    unsigned int hash = hash_function_servers(&main->hash_ring[pos]);

    int hmax = main->servers[src_id]->memory->hmax;
    for (int j = 0; j < hmax; j++) {
        ll_node_t *curr = main->servers[src_id]->memory->buckets[j]->head;

        while (curr) {
            char *key = (char*)(((info_t*)curr->data)->key);
            char key_copy[KEY_LENGTH] = {0};
            memcpy(key_copy, key, strlen(key)+1);
            unsigned int key_hash = hash_function_key(key);

            // cazul in care prev este ultimul elem, iar dest este primul
            if (pos == 0 && server_hash < hash) {
                if ((key_hash > prev_hash) || key_hash < server_hash) {
                    char *value = (char*)(((info_t*)curr->data)->value);
                    char value_copy[VALUE_LENGTH] = {0};
                    memcpy(value_copy, value, strlen(value) + 1);

                    curr = curr->next;

                    server_remove(main->servers[src_id], key_copy);
                    server_store(main->servers[dest_id], key_copy, value_copy);

                    continue;
                }
            } else if (key_hash < server_hash && key_hash > prev_hash) {
                char *value = (char*)(((info_t*)curr->data)->value);
                char value_copy[VALUE_LENGTH] = {0};
                memcpy(value_copy, value, strlen(value) + 1);

                curr = curr->next;

                server_remove(main->servers[src_id], key_copy);
                server_store(main->servers[dest_id], key_copy, value_copy);

                continue;
            }

            curr = curr->next;
        }
    }
}

/*
 *  Functia de realocare primeste etichate unei replici si gaseste urmatorul
 *  element din hashring (prin urmare, se obtine si cel anterior). Prin
 *  parametrul command se realizeaza diferenta dintre adaugarea si eliminarea
 *  unui server: in functie de acesta, se aleg serverul destinatie si serverul
 *  sursa. 
*/
static void reallocate_objects(load_balancer_t *main,
                                unsigned int server_tag, int command) {
    if (main->num_servers <= 3) {
        return;
    }
    unsigned int server_hash = hash_function_servers(&server_tag);
    int n = main->num_servers;
    int pos = binary_search(main->hash_ring, 0, n - 1, server_hash);
    int prev = pos - 1;

    if (pos == 0) {
        prev = n - 1;
    } else if (pos == n) {
        pos = 0;
    }

    // daca urmatoarea eticheta face parte din acelasi server, nu se realizeaza
    // realocarea, avand elementele stocate in aceeasi zona de memorie
    unsigned int next_id = main->hash_ring[pos] % MAX_SERVERS;
    if (server_tag % MAX_SERVERS == next_id) {
        return;
    }

    if (command == 1) {
        reallocate(main, pos, main->hash_ring[pos], server_tag,
                        main->hash_ring[prev], server_hash);
    } else {
        reallocate(main, pos, server_tag, main->hash_ring[pos],
                        main->hash_ring[prev], server_hash);
    }
}

/*
 *  Initializeazaa memoria pentru serverul de pe server_id si adauga pe rand in
 *  hashring etichetele, inainte realizand realocare pe eticheta respectiva.
*/
void loader_add_server(load_balancer_t* main, int server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return;
    }

    main->servers[server_id] = init_server_memory();
    reallocate_objects(main, server_id, 1);
    insert_in_order(main->hash_ring, MAX_HASH, &(main->num_servers), server_id);

    unsigned int rep1_tag = server_id + ID_REP;
    reallocate_objects(main, rep1_tag, 1);
    insert_in_order(main->hash_ring, MAX_HASH, &(main->num_servers), rep1_tag);

    unsigned int rep2_tag = server_id + ID_REP * 2;
    reallocate_objects(main, rep2_tag, 1);
    insert_in_order(main->hash_ring, MAX_HASH, &(main->num_servers), rep2_tag);
}

/* 
 *  Elimina serverul cu id-ul server_id din memoria dupa ce a sters din 
 *  hashring replicile si a realocata elementele fiecarea pe rand. 
*/
void loader_remove_server(load_balancer_t* main, int server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return;
    }

    unsigned int hash_server = hash_function_servers(&server_id);
    remove_in_order(main->hash_ring, &main->num_servers, hash_server);
    reallocate_objects(main, server_id, 2);

    unsigned int replica1_tag = server_id + ID_REP;
    unsigned int hash_rep1 = hash_function_servers(&replica1_tag);
    remove_in_order(main->hash_ring, &main->num_servers, hash_rep1);
    reallocate_objects(main, replica1_tag, 2);

    unsigned int replica2_tag = server_id + 2 * ID_REP;
    unsigned int hash_rep2 = hash_function_servers(&replica2_tag);
    remove_in_order(main->hash_ring, &main->num_servers, hash_rep2);
    reallocate_objects(main, replica2_tag, 2);

    free_server_memory(main->servers[server_id]);
    main->servers[server_id] = NULL;
}

/*  
 *  Functia care elibereaza memoria pentru load balancer si parametriis sai.
*/
void free_load_balancer(load_balancer_t* main) {
    free(main->hash_ring);
    for (int i = 0; i < MAX_HASH / 3; i++) {
        if (main->num_servers == 0) {
            break;
        }
        if (main->servers[i]) {
            free_server_memory(main->servers[i]);
            main->num_servers--;
        }
    }
    free(main->servers);
    free(main);
}
