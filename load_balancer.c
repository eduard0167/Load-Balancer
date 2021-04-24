/* Copyright 2021 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "Hashtable.h"

load_balancer_t* init_load_balancer() {
	load_balancer_t *main = malloc(sizeof(*main));
    DIE(!main, "Error in malloc() for load balancer\n");

    main->servers = malloc(100000 * sizeof(server_memory*));
    DIE(!main, "Error in malloc() for array of servers\n");
    
    main->hash_ring = malloc(3 * 100000 * sizeof(unsigned int));
    DIE(!main, "Error in malloc() for hash ring\n");

    main->num_servers = 0;
    main->hash_function_servers = hash_function_servers;
    main->hash_function_key = hash_function_key;
    
    return main;
}

void loader_store(load_balancer_t* main, char* key, char* value, int* server_id) {
    if (!main) {
        fprintf("Error: Load balancer hasn't been allocated\n");
        return;
    }

    if (main->num_servers == 0) {  // want to allocate first server

    }
}


char* loader_retrieve(load_balancer_t* main, char* key, int* server_id) {
	/* TODO. */
	return NULL;
}

void loader_add_server(load_balancer_t* main, int server_id) {
    if (!main) {
        fprintf("Error: Load balancer hasn't been allocated\n");
        return;
    }

    if (main->num_servers == 0) {  // want to allocate first server

    }
}


void loader_remove_server(load_balancer_t* main, int server_id) {
	/* TODO. */
}

void free_load_balancer(load_balancer_t* main) {
    /* TODO. */
}
