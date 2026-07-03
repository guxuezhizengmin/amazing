#include "hash_table.h"

/* BKDR字符串哈希算法 */
/*功能：BKDR 字符串哈希算法，将主键字符串映射为桶下标
逻辑：以 131 为种子逐字符迭代计算哈希值，对桶数取模得到数组下标*/
static unsigned int hash_string(const char *str) {
	unsigned int seed = 131;
	unsigned int hash = 0;
	while (*str) hash = hash * seed + (*str++);
	return hash % HASH_BUCKET_NUM;
}

HashTable* hash_table_create() {
	HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
	if (!ht) return NULL;
	memset(ht->buckets, 0, sizeof(ht->buckets));
	ht->count = 0;
	return ht;
}
/*功能：在指定桶的冲突链表中查找目标主键节点。*/
static HashNode* find_in_bucket(HashNode *head, const char *key) {
	HashNode *cur = head;
	char node_key[21];
	while (cur) {
		make_key(cur->data.student_id, cur->data.course_id, node_key);
		if (strcmp(key, node_key) == 0) return cur;
		cur = cur->next;
	}
	return NULL;
}

static int ht_insert(void *ds, CourseRecord record) {
	HashTable *ht = (HashTable*)ds;
	char key[21];
	make_key(record.student_id, record.course_id, key);
	unsigned int idx = hash_string(key);
	
	if (find_in_bucket(ht->buckets[idx], key)) return -1;
	
	HashNode *node = (HashNode*)malloc(sizeof(HashNode));
	if (!node) return -1;
	node->data = record;
	node->next = ht->buckets[idx];
	ht->buckets[idx] = node;
	ht->count++;
	return 0;
}

static int ht_remove(void *ds, const char *sid, const char *cid) {
	HashTable *ht = (HashTable*)ds;
	char key[21];
	make_key(sid, cid, key);
	unsigned int idx = hash_string(key);
	
	HashNode *cur = ht->buckets[idx];
	HashNode *prev = NULL;
	char node_key[21];
	
	while (cur) {
		make_key(cur->data.student_id, cur->data.course_id, node_key);
		if (strcmp(key, node_key) == 0) {
			if (prev) prev->next = cur->next;
			else ht->buckets[idx] = cur->next;
			free(cur);
			ht->count--;
			return 0;
		}
		prev = cur;
		cur = cur->next;
	}
	return -1;
}

static int ht_update(void *ds, const char *sid, const char *cid, int new_score) {
	char key[21];
	make_key(sid, cid, key);
	unsigned int idx = hash_string(key);
	HashNode *node = find_in_bucket(((HashTable*)ds)->buckets[idx], key);
	if (!node) return -1;
	node->data.score = new_score;
	return 0;
}

static CourseRecord* ht_find(void *ds, const char *sid, const char *cid) {
	char key[21];
	make_key(sid, cid, key);
	unsigned int idx = hash_string(key);
	HashNode *node = find_in_bucket(((HashTable*)ds)->buckets[idx], key);
	return node ? &(node->data) : NULL;
}

static void ht_traverse(void *ds, void (*cb)(CourseRecord, void*), void *arg) {
	HashTable *ht = (HashTable*)ds;
	for (int i = 0; i < HASH_BUCKET_NUM; i++) {
		HashNode *cur = ht->buckets[i];
		while (cur) {
			cb(cur->data, arg);
			cur = cur->next;
		}
	}
}

static int ht_size(void *ds) {
	return ((HashTable*)ds)->count;
}

static void ht_destroy(void *ds) {
	HashTable *ht = (HashTable*)ds;
	for (int i = 0; i < HASH_BUCKET_NUM; i++) {
		HashNode *cur = ht->buckets[i];
		while (cur) {
			HashNode *next = cur->next;
			free(cur);
			cur = next;
		}
	}
	free(ht);
}

DSInterface hash_table_get_interface() {
	DSInterface iface;
	iface.insert   = ht_insert;
	iface.remove   = ht_remove;
	iface.update   = ht_update;
	iface.find     = ht_find;
	iface.traverse = ht_traverse;
	iface.size     = ht_size;
	iface.destroy  = ht_destroy;
	return iface;
}
