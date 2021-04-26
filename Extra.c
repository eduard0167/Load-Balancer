#include "Extra.h"

void insert_in_order(unsigned int *hash_ring, int capacity, int *size, unsigned int server_tag) {
    if (*size >= capacity){
        return;
    }

    int i;
    for (i = *size - 1; (i >= 0 && hash_function_servers(&hash_ring[i]) > hash_function_servers(&server_tag)); i--) {
      	hash_ring[i + 1] = hash_ring[i];
    }
 
    hash_ring[i + 1] = server_tag;
	(*size)++;
}

int binary_search(unsigned int *arr, int n, int left, int right, unsigned int x) {
	int pos = -1;

	if (hash_function_servers(&arr[0]) > x) {
		return 0;
	}

	if (hash_function_servers(&arr[n - 1]) < x) {
		return 0;
	}

	while (left <= right) {
		int mid = (left + right) / 2;

		if (hash_function_servers(&arr[mid]) <= x) {
			left = mid + 1;
		} else {
			pos = mid;
			right = mid - 1;
		}
	}

	return pos;
}

int remove_search(unsigned int *arr, int n, int left, int right, unsigned int x) {
	int pos = -1;

	if (arr[0] >= x) {
		return 0;
	}

	if (arr[n - 1] <= x) {
		return n;
	}

	while (left <= right) {
		int mid = (left + right) / 2;

		if (arr[mid] <= x) {
			left = mid + 1;
		} else {
			pos = mid;
			right = mid - 1;
		}
	}

	return pos;
}

void remove_in_order(unsigned int *a, int *size, unsigned int x) {
	int pos = binary_search(a, *size, 0, *size - 1, x) - 1;

	for (int i = pos; i < *size - 1; i++) {
		a[i] = a[i + 1];
	}

	(*size)--;
} 