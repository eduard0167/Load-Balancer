/* Copyright 2021 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "Hashtable.h"
#include "Extra.h"
#include "utils.h"


load_balancer_t* init_load_balancer() {
	load_balancer_t *main = malloc(sizeof(*main));
    DIE(!main, "Error in malloc() for load balancer\n");

    main->servers = calloc(MAX_SERVERS, sizeof(server_memory*));
    DIE(!main, "Error in calloc() for array of servers\n");
 
    main->hash_ring = calloc(MAX_HASH, sizeof(unsigned int));
    DIE(!main, "Error in calloc() for hash ring\n");

    main->num_servers = 0;
    
    return main;
}

void loader_store(load_balancer_t* main, char* key, char* value, int* server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return;
    }

    unsigned int key_index = hash_function_key(key);
    int i = binary_search(main->hash_ring, main->num_servers, 0, main->num_servers - 1, key_index);

    *server_id = main->hash_ring[i] % MAX_SERVERS;
    server_store(main->servers[*server_id], key, value);
}

char* loader_retrieve(load_balancer_t* main, char* key, int* server_id) {
	unsigned int key_index = hash_function_key(key);
    int i = binary_search(main->hash_ring, main->num_servers, 0, main->num_servers - 1, key_index);

    *server_id = main->hash_ring[i] % MAX_SERVERS;
    char *value = server_retrieve(main->servers[*server_id], key);

	return value;
}

static void reallocate_objects(load_balancer_t *main, int server_id, unsigned int server_tag) {
    if (main->num_servers <= 3) {
        return;
    }
    unsigned int server_hash = hash_function_servers(&server_tag);
    int pos = binary_search(main->hash_ring, main->num_servers, 0, main->num_servers - 1, server_hash);
    unsigned int next_id = main->hash_ring[pos] % MAX_SERVERS;
    if (server_tag % MAX_SERVERS == next_id) {
        return;
    }

    int prev;
    if (pos != 0) {
        prev = pos - 1; 
    } else {
        prev = main->num_servers - 1;
    }

    int hmax = main->servers[next_id]->memory->hmax;
    for (int j = 0; j < hmax; j++) {
        ll_node_t *curr = main->servers[next_id]->memory->buckets[j]->head;

        while (curr) {
            char *key = (char*)(((info_t*)curr->data)->key);
            char key_copy[KEY_LENGTH] = {0};
            memcpy(key_copy, key, strlen(key)+1);
            unsigned int key_hash = hash_function_key(key);

            if (key_hash < server_hash && key_hash > hash_function_servers(&main->hash_ring[prev])) {
                char *value = (char*)(((info_t*)curr->data)->value);
                char value_copy[VALUE_LENGTH] = {0};
                memcpy(value_copy, value, strlen(value) + 1);

                curr = curr->next;

                server_remove(main->servers[next_id], key_copy);
                server_store(main->servers[server_id], key_copy, value_copy);

                continue;
            }

            curr = curr->next;
        }
    }
}

void loader_add_server(load_balancer_t* main, int server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return;
    }

    main->servers[server_id] = init_server_memory();
    main->servers[server_id]->id_server = server_id;
    reallocate_objects(main, server_id, server_id);
    insert_in_order(main->hash_ring, MAX_HASH, &(main->num_servers), server_id);

    unsigned int replica1_tag = server_id + ID_REP;
    reallocate_objects(main, server_id, replica1_tag);
    insert_in_order(main->hash_ring, MAX_HASH, &(main->num_servers), replica1_tag);
    
    unsigned int replica2_tag = server_id + ID_REP * 2;
    reallocate_objects(main, server_id, replica2_tag);
    insert_in_order(main->hash_ring, MAX_HASH, &(main->num_servers), replica2_tag);
}


static void reallocate_remove(load_balancer_t *main, int server_id, unsigned int server_tag) {
    if (main->num_servers <= 3) {
        return;
    }

    unsigned int server_hash = hash_function_servers(&server_tag);
    int pos = binary_search(main->hash_ring, main->num_servers, 0, main->num_servers - 1, server_hash);
    unsigned int next_id = main->hash_ring[pos] % MAX_SERVERS;
    server_hash = hash_function_servers(&main->hash_ring[pos]);
    if (server_tag % MAX_SERVERS == next_id) {
        return;
    }
    
    int prev;
    if (pos != 0) {
        prev = pos - 1; 
    } else {
        prev = main->num_servers - 1;
    }

    int hmax = main->servers[server_id]->memory->hmax;
    for (int j = 0; j < hmax; j++) {
        ll_node_t *curr = main->servers[server_id]->memory->buckets[j]->head;

        while (curr) {
            char *key = (char*)(((info_t*)curr->data)->key);
            char key_copy[KEY_LENGTH] = {0};
            memcpy(key_copy, key, strlen(key)+1);
            unsigned int key_hash = hash_function_key(key);
            
            if (key_hash < server_hash && key_hash > hash_function_servers(&main->hash_ring[prev])) {
                char *value = (char*)(((info_t*)curr->data)->value);
                char value_copy[VALUE_LENGTH] = {0};
                memcpy(value_copy, value, strlen(value) + 1);

                curr = curr->next;

                server_remove(main->servers[server_id], key_copy);
                server_store(main->servers[next_id], key_copy, value_copy);

                continue;
            }

            curr = curr->next;
        }
    }
}

void loader_remove_server(load_balancer_t* main, int server_id) {
    if (!main) {
        fprintf(stderr, "Error: Load balancer hasn't been allocated\n");
        return;
    }

    unsigned int hash_server = hash_function_servers(&server_id);
    remove_in_order(main->hash_ring, &main->num_servers, hash_server);
    reallocate_remove(main, server_id, server_id);
    
    unsigned int replica1_tag = server_id + ID_REP;
    unsigned int hash_rep1 = hash_function_servers(&replica1_tag);
    remove_in_order(main->hash_ring, &main->num_servers, hash_rep1);
    reallocate_remove(main, server_id, replica1_tag);

    unsigned int replica2_tag = server_id + 2 * ID_REP; 
    unsigned int hash_rep2 = hash_function_servers(&replica2_tag);
    remove_in_order(main->hash_ring, &main->num_servers, hash_rep2);
    reallocate_remove(main, server_id, replica2_tag);

    free_server_memory(main->servers[server_id]);
    main->servers[server_id] = NULL;
}

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
