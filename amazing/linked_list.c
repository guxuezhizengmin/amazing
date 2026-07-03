#include "linked_list.h"
/*创建并初始化一个空的双向链表*/
LinkedList* linked_list_create() {
	LinkedList *list = (LinkedList*)malloc(sizeof(LinkedList));
	if (!list) return NULL;
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;
	return list;
}
/*成功返回 0，主键重复返回 - 1*/
/* 内部：按学号+课程号查找节点 */
static ListNode* find_node(LinkedList *list, const char *sid, const char *cid) {
	ListNode *cur = list->head;
	char key_target[21], key_node[21];
	make_key(sid, cid, key_target);
	while (cur) {
		make_key(cur->data.student_id, cur->data.course_id, key_node);
		if (strcmp(key_target, key_node) == 0) return cur;
		cur = cur->next;
	}
	return NULL;
}
/*尾插法插入一条新记录，插入前自动查重*/
static int ll_insert(void *ds, CourseRecord record) {
	LinkedList *list = (LinkedList*)ds;
	if (find_node(list, record.student_id, record.course_id)) return -1;
	
	ListNode *node = (ListNode*)malloc(sizeof(ListNode));
	if (!node) return -1;
	node->data = record;
	node->prev = list->tail;
	node->next = NULL;
	
	if (list->tail) list->tail->next = node;
	else list->head = node;
	list->tail = node;
	list->count++;
	return 0;
}
/*根据学号 + 课程号删除指定记录*/
static int ll_remove(void *ds, const char *sid, const char *cid) {
	LinkedList *list = (LinkedList*)ds;
	ListNode *node = find_node(list, sid, cid);
	if (!node) return -1;
	
	if (node->prev) node->prev->next = node->next;
	else list->head = node->next;
	if (node->next) node->next->prev = node->prev;
	else list->tail = node->prev;
	
	free(node);
	list->count--;
	return 0;
}
/*修改指定记录的成绩字段*/
static int ll_update(void *ds, const char *sid, const char *cid, int new_score) {
	ListNode *node = find_node((LinkedList*)ds, sid, cid);
	if (!node) return -1;
	node->data.score = new_score;
	return 0;
}
/*精确查找指定选课记录*/
static CourseRecord* ll_find(void *ds, const char *sid, const char *cid) {
	ListNode *node = find_node((LinkedList*)ds, sid, cid);
	return node ? &(node->data) : NULL;
}
/*全量顺序遍历链表，对每条记录调用回调函数*/
static void ll_traverse(void *ds, void (*cb)(CourseRecord, void*), void *arg) {
	ListNode *cur = ((LinkedList*)ds)->head;
	while (cur) {
		cb(cur->data, arg);
		cur = cur->next;
	}
}
/*返回当前链表的记录总数*/
static int ll_size(void *ds) {
	return ((LinkedList*)ds)->count;
}
/*销毁链表，逐节点释放全部内存，避免泄漏*/
static void ll_destroy(void *ds) {
	LinkedList *list = (LinkedList*)ds;
	ListNode *cur = list->head;
	while (cur) {
		ListNode *next = cur->next;
		free(cur);
		cur = next;
	}
	free(list);
}
/*返回填充好函数指针的统一接口结构体*/
DSInterface linked_list_get_interface() {
	DSInterface iface;
	iface.insert   = ll_insert;
	iface.remove   = ll_remove;
	iface.update   = ll_update;
	iface.find     = ll_find;
	iface.traverse = ll_traverse;
	iface.size     = ll_size;
	iface.destroy  = ll_destroy;
	return iface;
}
