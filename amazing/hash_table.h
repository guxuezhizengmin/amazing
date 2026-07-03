#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common.h"

#define HASH_BUCKET_NUM 10007  /* 质数桶数，减少冲突 */

typedef struct HashNode {
	CourseRecord data;
	struct HashNode *next;
} HashNode;

typedef struct {
	HashNode *buckets[HASH_BUCKET_NUM];
	int count;
} HashTable;

HashTable* hash_table_create();
DSInterface hash_table_get_interface();

#endif
