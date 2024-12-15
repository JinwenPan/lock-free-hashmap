// Reference: 
// https://gist.github.com/ionuttamas/c8c2333e36e7570ae1aa
// https://secondboyet.com/Articles/LockFreeLinkedList3.html
// https://people.csail.mit.edu/bushl2/rpi/project_web/page5.html
// https://github.com/pramalhe/ConcurrencyFreaks/blob/master/Java/com/concurrencyfreaks/list/HarrisAMRLinkedList.java
// https://github.com/gpiskas/LinkedLists_Unlocked/blob/master/src/linkedlist/linkedlist.c
// https://github.com/bhhbazinga/LockFreeLinkedList/blob/master/lockfree_linkedlist.h
// Lock-Free Linked Lists and Skip Lists, Mikhail Fomitchev and Eric Ruppert
// LOCK-FREE LINKED LISTS AND SKIP LISTS, master's thesis, MIKHAIL FOMITCHEV
// The Art of Multiprocessor Programming, Maurice Herlihy and Nir Shavit
// A Pragmatic Implementation of Non-Blocking Linked-Lists, Timothy L. Harris

#include "chashmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

typedef struct Node_HM_t
{
	long m_val;
	char padding[PAD];
	Node_HM* m_next;
} Node_HM;

typedef struct List_t
{
	Node_HM* sentinel; 
	Node_HM* tail;
} List;

typedef struct hm_t
{
    List** buckets; 
	size_t num_buckets;
} HM;

// mark means modify LSB in next field of cur node(it's the memory address of next node not cur node)
uintptr_t is_marked(Node_HM* p){
	uintptr_t i = (uintptr_t)p;
	uintptr_t j = (uintptr_t)0x1UL;
	return (i & j);
}

Node_HM* read_unmarked(Node_HM* p){
	uintptr_t i = (uintptr_t)p;
	uintptr_t j = (uintptr_t)0x1UL;
	j = ~j;
	i &= j;
	Node_HM* n = (Node_HM*)i;
	return n;
}

Node_HM* read_marked(Node_HM* p){
	uintptr_t i = (uintptr_t)p;
	uintptr_t j = (uintptr_t)0x1UL;
	i |= j;
	Node_HM* n = (Node_HM*)i;
	return n;
}

HM* alloc_hashmap(size_t n_buckets){
	HM* hash_map = (HM*)malloc(sizeof(HM));
    if (hash_map == NULL) return NULL;

	hash_map->num_buckets = n_buckets;

	hash_map->buckets = (List**)malloc(sizeof(List*) * n_buckets);
	if (hash_map->buckets == NULL) {
		free(hash_map);
		return NULL;
	}

	for (size_t i = 0; i < n_buckets; i++){
		hash_map->buckets[i] = (List*)malloc(sizeof(List));
		if (hash_map->buckets[i] == NULL){
			for (size_t j = 0; j < i; j++){
				free(hash_map->buckets[j]->sentinel);
				free(hash_map->buckets[j]->tail);
				free(hash_map->buckets[j]);	
			}
			free(hash_map->buckets);
			free(hash_map);
			return NULL;
		}
		hash_map->buckets[i]->sentinel = (Node_HM*)malloc(sizeof(Node_HM));
		if (hash_map->buckets[i]->sentinel == NULL){
			for (size_t j = 0; j < i; j++){
				free(hash_map->buckets[j]->sentinel);
				free(hash_map->buckets[j]->tail);
				free(hash_map->buckets[j]);	
			}
			free(hash_map->buckets[i]);
			free(hash_map->buckets);
			free(hash_map);
			return NULL;
		}
		hash_map->buckets[i]->tail = (Node_HM*)malloc(sizeof(Node_HM));
		if (hash_map->buckets[i]->tail == NULL){
			for (size_t j = 0; j < i; j++){
				free(hash_map->buckets[j]->sentinel);
				free(hash_map->buckets[j]->tail);
				free(hash_map->buckets[j]);	
			}
			free(hash_map->buckets[i]->sentinel);
			free(hash_map->buckets[i]);
			free(hash_map->buckets);
			free(hash_map);
			return NULL;
		}
		
		hash_map->buckets[i]->sentinel->m_next = hash_map->buckets[i]->tail;
		hash_map->buckets[i]->sentinel->m_val = LONG_MIN;
		hash_map->buckets[i]->tail->m_next = NULL;
		hash_map->buckets[i]->tail->m_val = LONG_MAX;
	}

	return hash_map;
}

void free_hashmap(HM* hm){
	if (hm == NULL) return ;
	
	for (size_t i = 0; i < hm->num_buckets; i++){
		Node_HM* cur_node = hm->buckets[i]->sentinel;
		Node_HM* freed_node = NULL;
		while (cur_node != NULL){
			freed_node = read_unmarked(cur_node);
			cur_node = read_unmarked(cur_node->m_next);
			free(freed_node);
		}
		free(hm->buckets[i]);
	}
	free(hm->buckets);
	free(hm);
}

Node_HM* get_nodes_window(List* l, long val, Node_HM** pre){
	Node_HM* cur = NULL;
	Node_HM* pre_suc = NULL;

	while (1) {
		Node_HM* t = l->sentinel;
		Node_HM* t_suc = l->sentinel->m_next;
		do {
			if (!is_marked(t_suc)){
				(*pre) = t;
				pre_suc = t_suc;
			}
			t = read_unmarked(t_suc);
			if (t == l->tail) break;
			t_suc = t->m_next;
		} while(is_marked(t_suc) || t->m_val < val);
		cur = t;

		if (pre_suc == cur){
			if (cur != l->tail && is_marked(cur->m_next)) continue;
			else return cur;
		}

		if (__sync_bool_compare_and_swap(&((*pre)->m_next), pre_suc, cur)){
			if (cur != l->tail && is_marked(cur->m_next)) continue;
			else return cur;
		}
	}
}

int insert_item(HM* hm, long val){
	if (hm == NULL) return 1;

	long bucket_index = val % hm->num_buckets;

	Node_HM* new_node = (Node_HM*)malloc(sizeof(Node_HM));
	if (new_node == NULL) return 1;
	new_node->m_val = val;

	while (1) {
		Node_HM* pre;
		Node_HM* cur;
        cur = get_nodes_window(hm->buckets[bucket_index], val, &pre);
        new_node->m_next = cur; 
        if (__sync_bool_compare_and_swap(&(pre->m_next), cur, new_node)) return 0;
    }
}

int remove_item(HM* hm, long val){
	if (hm == NULL) return 1;
	
	long bucket_index = val % hm->num_buckets;
    
	Node_HM* pre;
    Node_HM* cur;
	Node_HM* suc;

    while (1) {
        cur = get_nodes_window(hm->buckets[bucket_index], val, &pre);
		if (cur == hm->buckets[bucket_index]->tail || cur->m_val != val) return 1;
        suc = cur->m_next;
		if (!is_marked(suc)) {
			if (__sync_bool_compare_and_swap(&(cur->m_next), suc, read_marked(suc))) {
				break;
			}
		}
	}

	if(!__sync_bool_compare_and_swap(&(pre->m_next), cur, suc)){
		cur = get_nodes_window(hm->buckets[bucket_index], cur->m_val, &pre);
	}

	return 0;
}

int lookup_item(HM* hm, long val){
	if (hm == NULL) return 1;
	
	long bucket_index = val % hm->num_buckets;
	
	Node_HM* pre;
	Node_HM* cur;
	cur = get_nodes_window(hm->buckets[bucket_index], val, &pre);
	if (cur == hm->buckets[bucket_index]->tail || cur->m_val != val || is_marked(cur->m_next)) return 1;
	else return 0;
}

//print all elements in the hashmap as follows:
//Bucket 1 - val1 - val2 - val3 ...
//Bucket 2 - val4 - val5 - val6 ...
//Bucket N -  ...
void print_hashmap(HM* hm){
	if (hm == NULL) return ;
	
	for (size_t i = 0; i < hm->num_buckets; i++){
		printf("Bucket %zu ", i+1);
		Node_HM* cur_node = read_unmarked(hm->buckets[i]->sentinel->m_next);
		if (cur_node == hm->buckets[i]->tail){
			printf("-  \n");
		}
		else{
			while (cur_node != hm->buckets[i]->tail){
				if(!is_marked(cur_node->m_next)) printf("- %ld ", cur_node->m_val);
				cur_node = read_unmarked(cur_node->m_next);
			}
			printf("\n");
		}
	}
}