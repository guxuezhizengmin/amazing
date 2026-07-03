#include "perf_test.h"
#include "data_gen.h"
#include "linked_list.h"
#include "avl_tree.h"
#include "hash_table.h"
#include "filter_sort.h"

//遍历计数回调，验证遍历完整性
static void count_cb(CourseRecord r, void *arg) { (*(int*)arg)++; }

typedef struct { CourseRecord *arr; int idx; } CollectArg;

//遍历收集回调，将记录导出到数组，用于排序测试
static void collect_cb(CourseRecord r, void *arg) {
	CollectArg *ca = (CollectArg*)arg;
	ca->arr[ca->idx++] = r;
}

//获取当前毫秒级时间戳，基于标准库 clock 实现
static double get_ms() {
	return (double)clock() / CLOCKS_PER_SEC * 1000.0;
}

//测试批量插入的总耗时
static double test_insert(void *ds, DSInterface iface, CourseRecord *recs, int n) {
	double start = get_ms();
	for (int i = 0; i < n; i++) iface.insert(ds, recs[i]);
	return get_ms() - start;
}

//测试单次随机查找的平均耗时（微秒级），随机选取 100 条测试取平均
static double test_find_us(void *ds, DSInterface iface, CourseRecord *recs, int n) {
	int times = n < 100 ? n : 100;
	double start = get_ms();
	for (int i = 0; i < times; i++) {
		int idx = rand() % n;
		iface.find(ds, recs[idx].student_id, recs[idx].course_id);
	}
	return (get_ms() - start) / times * 1000.0;
}

//测试单次随机删除的平均耗时，测试后重新插回保证数据量不变
static double test_remove_us(void *ds, DSInterface iface, CourseRecord *recs, int n) {
	int times = n < 100 ? n : 100;
	for (int i = 0; i < times; i++) iface.insert(ds, recs[i]);
	
	double start = get_ms();
	for (int i = 0; i < times; i++)
		iface.remove(ds, recs[i].student_id, recs[i].course_id);
	double t = get_ms() - start;
	
	for (int i = 0; i < times; i++) iface.insert(ds, recs[i]);
	return t / times * 1000.0;
}
//测试全量遍历的总耗时
static double test_traverse_ms(void *ds, DSInterface iface) {
	int cnt = 0;
	double start = get_ms();
	iface.traverse(ds, count_cb, &cnt);
	return get_ms() - start;
}
//测试按成绩排序的总耗时，先导出数据再排序
static double test_sort_ms(void *ds, DSInterface iface) {
	int n = iface.size(ds);
	if (n <= 0) return 0;
	
	CourseRecord *recs = malloc(n * sizeof(CourseRecord));
	if (!recs) return 0;
	
	CollectArg ca = {recs, 0};
	iface.traverse(ds, collect_cb, &ca);
	
	SortRule rule = {SORT_SCORE, 1};
	double start = get_ms();
	sort_records(recs, n, &rule, 1);
	double t = get_ms() - start;
	
	free(recs);
	return t;
}
//根据数据结构类型估算内存占用，基于节点大小与记录数理论计算
static double est_mem(int n, int type) {
	double per;
	switch(type) {
		case 0: per = sizeof(CourseRecord) + 2*sizeof(void*); break;
		case 1: per = sizeof(CourseRecord) + 2*sizeof(void*) + sizeof(int); break;
		case 2: per = sizeof(CourseRecord) + sizeof(void*);
		per += (HASH_BUCKET_NUM * sizeof(void*)) / (double)n; break;
		default: per = sizeof(CourseRecord);
	}
	return per * n / 1024.0 / 1024.0;
}
/*功能：执行完整的三结构性能对比测试
逻辑：生成统一测试数据集，分别在三种结构上执行所有操作测试，最后格式化输出对比表格；
测试完成后自动释放全部资源，无内存泄漏。*/
void run_performance_test(int record_count) {
	if (record_count <= 0) {
		printf("[错误] 记录条数必须大于0\n");
		return;
	}
	
	printf("\n===== 性能对比测试（%d 条记录） =====\n", record_count);
	CourseRecord *recs = generate_records(record_count);
	if (!recs) {
		printf("[错误] 数据生成失败，内存不足\n");
		return;
	}
	
	LinkedList *list = linked_list_create();
	AVLTree *avl = avl_tree_create();
	HashTable *ht = hash_table_create();
	DSInterface li = linked_list_get_interface();
	DSInterface ai = avl_tree_get_interface();
	DSInterface hi = hash_table_get_interface();
	
	double ins_l = test_insert(list, li, recs, record_count);
	double ins_a = test_insert(avl, ai, recs, record_count);
	double ins_h = test_insert(ht, hi, recs, record_count);
	
	double find_l = test_find_us(list, li, recs, record_count);
	double find_a = test_find_us(avl, ai, recs, record_count);
	double find_h = test_find_us(ht, hi, recs, record_count);
	
	double del_l = test_remove_us(list, li, recs, record_count);
	double del_a = test_remove_us(avl, ai, recs, record_count);
	double del_h = test_remove_us(ht, hi, recs, record_count);
	
	double trav_l = test_traverse_ms(list, li);
	double trav_a = test_traverse_ms(avl, ai);
	double trav_h = test_traverse_ms(ht, hi);
	
	double sort_l = test_sort_ms(list, li);
	double sort_a = test_sort_ms(avl, ai);
	double sort_h = test_sort_ms(ht, hi);
	
	double mem_l = est_mem(record_count, 0);
	double mem_a = est_mem(record_count, 1);
	double mem_h = est_mem(record_count, 2);
	
	printf("\n%-12s %-12s %-12s %-12s\n", "操作", "双向链表", "AVL树", "哈希表");
	printf("------------------------------------------------------------\n");
	printf("%-12s %-10.2fms %-10.2fms %-10.2fms\n", "插入总耗时", ins_l, ins_a, ins_h);
	printf("%-12s %-10.2fμs %-10.2fμs %-10.2fμs\n", "单次查找", find_l, find_a, find_h);
	printf("%-12s %-10.2fμs %-10.2fμs %-10.2fμs\n", "单次删除", del_l, del_a, del_h);
	printf("%-12s %-10.2fms %-10.2fms %-10.2fms\n", "全量遍历", trav_l, trav_a, trav_h);
	printf("%-12s %-10.2fms %-10.2fms %-10.2fms\n", "成绩排序", sort_l, sort_a, sort_h);
	printf("%-12s %-10.2fMB %-10.2fMB %-10.2fMB\n", "内存占用", mem_l, mem_a, mem_h);
	printf("------------------------------------------------------------\n");
	
	li.destroy(list);
	ai.destroy(avl);
	hi.destroy(ht);
	free(recs);
	printf("\n测试完成\n");
}
