/* Copyright 2021 Mitroi Eduard Ionut */
#include "server.h"

#define MAX_ELEM 1024

server_memory* init_server_memory() {
	server_memory *server = malloc(sizeof(*server));
	DIE(!server, "Erroc in malloc() for server\n");

	server->memory = ht_create(MAX_ELEM, hash_function_key,
															compare_function_strings);
	return server;
}

void server_store(server_memory* server, char* key, char* value) {
	ht_put(server->memory, key, strlen(key) + 1, value, strlen(value) + 1);
}

void server_remove(server_memory* server, char* key) {
	ht_remove_entry(server->memory, key);
}

char* server_retrieve(server_memory* server, char* key) {
	char *product = ht_get(server->memory, key);

	return product;
}

void free_server_memory(server_memory* server) {
	ht_free(server->memory);
	free(server);
}
