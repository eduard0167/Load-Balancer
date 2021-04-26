#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"

#define MAX_BUCKET_SIZE 128

/*
 * Functii de comparare a cheilor:
 */
int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functii de hashing:
 */
unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
hashtable_t *ht_create(int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	hashtable_t *ht = malloc(sizeof(hashtable_t));
  DIE(!ht, "malloc() for hashtable failed\n");
	
	ht->buckets = malloc(hmax * sizeof(linked_list_t *));
  DIE(!ht->buckets, "malloc() for buckets failed\n");

	for (int i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(info_t));
		DIE(!ht->buckets, "malloc() for buckets[i] failed\n");
	}

	ht->hmax = hmax;
	ht->size = 0;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
 
	return ht;
}

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune tipul ei), in momentul in care
 * se creeaza o noua intrare in hashtable (in cazul in care cheia nu se gaseste deja in ht), trebuie creata o copie
 * a valorii la care pointeaza key si adresa acestei copii trebuie salvata in structura info asociata intrarii din ht.
 * Pentru a sti cati octeti trebuie alocati si copiati, folositi parametrul key_size_bytes.
 *
 * Motivatie:
 * Este nevoie sa copiem valoarea la care pointeaza key deoarece dupa un apel put(ht, key_actual, value_actual),
 * valoarea la care pointeaza key_actual poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia unei intrari din hashtable.
 * Nu ne dorim acest lucru, fiindca exista riscul sa ajungem in situatia in care nu mai stim la ce cheie este
 * inregistrata o anumita valoare.
 */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *curr = ht->buckets[index]->head;
	while (curr) {
		if (ht->compare_function(((info_t *)curr->data)->key, key) == 0) {
			memcpy(((info_t*)curr->data)->value, value, value_size);
			return;
		}
		curr = curr->next;
	}

	if (!curr) {
		info_t new;
		new.key = malloc(key_size);
		memcpy(new.key, key, key_size);

		new.value = malloc(value_size);
		memcpy(new.value, value, value_size);

		ll_add_nth_node(ht->buckets[index], 0, &new);
	}

	ht->size++;
}

void *ht_get(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *curr = ht->buckets[index]->head;
	while (curr) {
		if (ht->compare_function(((info_t *)curr->data)->key, key) == 0) {
			return ((info_t *)curr->data)->value;
		}
		curr = curr->next;
	}

	return NULL;
}

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable folosind functia put
 * 0, altfel.
 */
int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *curr = ht->buckets[index]->head;
	while (curr) {
		if (ht->compare_function(((info_t *)curr->data)->key, key) == 0) {
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o intrare din hashtable (adica memoria
 * pentru copia lui key --vezi observatia de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */
void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *curr = ht->buckets[index]->head;
	unsigned int count = 0;

	while (curr) {
		if (ht->compare_function(((info_t *)curr->data)->key, key) == 0) {
			ll_node_t *remove = ll_remove_nth_node(ht->buckets[index], count);
			free(((info_t *)remove->data)->key);
			free(((info_t *)remove->data)->value);
			free(remove->data);
			free(remove);
			
			ht->size--;
			return;
		}

		curr = curr->next;
		count++;
	}
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable, dupa care elibereaza si memoria folosita
 * pentru a stoca structura hashtable.
 */
static void free_buckets(linked_list_t **pp_bucket)
{
    if (pp_bucket == NULL || *pp_bucket == NULL) {
        return;
    }

    ll_node_t* curr = NULL;

    while (ll_get_size(*pp_bucket) > 0) {
        curr = ll_remove_nth_node(*pp_bucket, 0);
        free(((info_t *)curr->data)->key);
        free(((info_t *)curr->data)->value);
        free(curr->data);
        free(curr);
    }

    free(*pp_bucket);
    *pp_bucket = NULL;
}

void ht_free(hashtable_t *ht)
{
	for (int i = 0; i < ht->hmax; i++) {
		free_buckets(&ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL) {
		return 0;
	}

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL) {
		return 0;
	}

	return ht->hmax;
}
