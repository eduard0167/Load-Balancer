/* Copyright 2021 Mitroi Eduard Ionut */
#ifndef ARRAY_H_
#define ARRAY_H_

#include "load_balancer.h"

void insert_in_order(unsigned int *hash_ring, int capacity, int *size,
                      unsigned int server_tag);
int binary_search(unsigned int *arr, int left, int right, unsigned int x);
void remove_in_order(unsigned int *a, int *size, unsigned int x);

#endif  /* ARRAY_H_ */
