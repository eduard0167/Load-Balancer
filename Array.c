/* Copyright 2021 Mitroi Eduard Ionut */
#include "Array.h"

void insert_in_order(unsigned int *hash_ring, int capacity,
											int *size, unsigned int server_tag) {
    if (*size >= capacity){
        return;
    }

    int i = *size - 1;
		unsigned int hash_tag = hash_function_servers(&server_tag);

    while (i >= 0) {
				unsigned int curr_hash = hash_function_servers(&hash_ring[i]);
				if (curr_hash < hash_tag) {
					break;
				}
      	hash_ring[i + 1] = hash_ring[i];
				i--;
    }

  hash_ring[i + 1] = server_tag;
	(*size)++;
}

int binary_search(unsigned int *arr, int left, int right, unsigned int x) {
	if (hash_function_servers(&arr[0]) > x) {
		return 0;
	}

	if (hash_function_servers(&arr[right]) < x) {
		return right + 1;
	}

	while (left <= right) {
		int mid = (left + right) / 2;

		if (hash_function_servers(&arr[mid]) <= x) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	return left;
}

void remove_in_order(unsigned int *a, int *size, unsigned int x) {
	int pos = binary_search(a, 0, *size - 1, x) - 1;

	for (int i = pos; i < *size - 1; i++) {
		a[i] = a[i + 1];
	}

	(*size)--;
}
