/* Copyright 2021 <> */
#ifndef EXTRA_H_
#define EXTRA_H_

#include "load_balancer.h"

void insert_in_order(unsigned int *hash_ring, int capacity, int *size, unsigned int server_tag);
int binary_search(unsigned int *arr, int n, int left, int right, unsigned int x);

#endif  /* EXTRA_H_ */
