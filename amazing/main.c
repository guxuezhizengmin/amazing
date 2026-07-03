#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "linked_list.h"
#include "avl_tree.h"
#include "hash_table.h"
#include "data_gen.h"
#include "persistence.h"
#include "filter_sort.h"
#include "stats.h"
#include "perf_test.h"

// 新增：Windows 平台下引入控制台 API
#ifdef _WIN32
#include <windows.h>
#endif

typedef enum { DS_LIST, DS_AVL, DS_HASH } DSType;
void *g_ds = NULL;
DSInterface g_iface;
DSType g_type;

typedef struct { CourseRecord *arr; int idx; } CollectArg;
//遍历收集回调，将记录导出到数组，用于排序功能
static void collect_cb(CourseRecord r, void *arg) {
	CollectArg *ca = (CollectArg*)arg;
	ca->arr[ca->idx++] = r;
}
//遍历统计过期记录数量，用于批量删除前的数量提示
static void count_expired_cb(CourseRecord r, void *arg) {
	if (compare_date(r.enroll_date, "2023-09-01") < 0) {
		(*(int*)arg)++;
	}
}

typedef struct { char (*keys)[21]; int idx; } ExpireArg;
//遍历收集所有过期记录的主键，用于后续批量删除（避免遍历中修改结构）
static void collect_expire_cb(CourseRecord r, void *arg) {
	ExpireArg *ea = (ExpireArg*)arg;
	if (compare_date(r.enroll_date, "2023-09-01") < 0) {
		make_key(r.student_id, r.course_id, ea->keys[ea->idx]);
		ea->idx++;
	}
}
//初始化指定类型的数据结构，自动销毁旧结构，切换存储时自动清理
int init_ds(DSType type) {
	if (g_ds) g_iface.destroy(g_ds);
	switch(type) {
		case DS_LIST: g_ds = linked_list_create(); g_iface = linked_list_get_interface(); break;
		case DS_AVL:  g_ds = avl_tree_create();    g_iface = avl_tree_get_interface();    break;
		case DS_HASH: g_ds = hash_table_create();  g_iface = hash_table_get_interface();  break;
		default: return -1;
	}
	g_type = type;
	return g_ds ? 0 : -1;
}
//印记录表格的表头
void print_header() {
	printf("%-14s %-10s %-22s %-10s %-18s %-5s %-8s %-12s %-4s\n",
		   "学号", "姓名", "学院", "课程号", "课程名", "学分", "学期", "选课日期", "成绩");
	printf("----------------------------------------------------------------------------------------------------\n");
}
//格式化打印单条选课记录
void print_rec(CourseRecord r) {
	printf("%-14s %-10s %-22s %-10s %-18s %-5.1f %-8s %-12s %-4d\n",
		   r.student_id, r.name, r.college, r.course_id, r.course_name,
		   r.credit, r.semester, r.enroll_date, r.score);
}
//打印主菜单，展示当前结构类型与记录总数
void show_menu() {
	printf("\n========== 选课记录管理系统 ==========\n");
	printf("当前结构：");
	switch(g_type) {
		case DS_LIST: printf("双向链表"); break;
		case DS_AVL:  printf("AVL树"); break;
		case DS_HASH: printf("哈希表"); break;
	}
	printf(" | 记录数：%d\n", g_iface.size(g_ds));
	printf("----------------------------------------\n");
	printf(" 1. 切换数据结构   2. 生成测试数据\n");
	printf(" 3. 加载CSV文件    4. 新增记录\n");
	printf(" 5. 删除记录       6. 修改成绩\n");
	printf(" 7. 精确查找       8. 多条件筛选\n");
	printf(" 9. 多关键字排序   10. 统计分析\n");
	printf("11. 批量删过期     12. 性能对比测试\n");
	printf("13. 保存到CSV      0. 退出\n");
}

void switch_ds() {
	printf("\n1-双向链表  2-AVL树  3-哈希表\n");
	int c = safe_input_int(1, 3, "请选择目标结构：");
	
	if (g_iface.size(g_ds) > 0) {
		if (!safe_confirm("切换将清空当前所有数据，确认继续？(Y/N)：")) {
			printf("已取消切换\n");
			return;
		}
	}
	init_ds(c - 1);
	printf("[成功] 切换完成，当前数据已清空\n");
}

void gen_data() {
	int n = safe_input_int(1, 100000, "请输入生成条数（1-100000）：");
	CourseRecord *recs = generate_records(n);
	if (!recs) {
		printf("[错误] 数据生成失败，内存不足\n");
		return;
	}
	
	init_ds(g_type);
	int success = 0;
	for (int i = 0; i < n; i++) {
		if (g_iface.insert(g_ds, recs[i]) == 0) success++;
	}
	free(recs);
	printf("[成功] 生成完成，成功插入 %d 条记录\n", success);
}

void load_file() {
	char fn[256];
	safe_input_str(fn, sizeof(fn), "请输入CSV文件名：");
	
	int n = load_from_csv(g_ds, g_iface, fn);
	if (n >= 0) {
		printf("[成功] 加载完成，共导入 %d 条记录\n", n);
	} else {
		printf("[错误] 加载失败，请检查文件是否存在、路径是否正确\n");
	}
}

void add_rec() {
	CourseRecord r;
	memset(&r, 0, sizeof(r));
	
	safe_input_str(r.student_id, sizeof(r.student_id), "学号（12位）：");
	safe_input_str(r.name, sizeof(r.name), "姓名：");
	safe_input_str(r.college, sizeof(r.college), "学院：");
	safe_input_str(r.course_id, sizeof(r.course_id), "课程编号（8位）：");
	safe_input_str(r.course_name, sizeof(r.course_name), "课程名称：");
	r.credit = safe_input_int(10, 80, "学分（整数，单位0.1，输入35代表3.5）：") / 10.0f;
	safe_input_str(r.semester, sizeof(r.semester), "选课学期（如2024-02）：");
	safe_input_str(r.enroll_date, sizeof(r.enroll_date), "选课日期（如2024-09-01）：");
	r.score = safe_input_int(0, 100, "成绩（0-100）：");
	
	if (g_iface.insert(g_ds, r) == 0) {
		printf("[成功] 添加记录成功\n");
	} else {
		printf("[错误] 添加失败，该学号+课程编号已存在\n");
	}
}

void del_rec() {
	char sid[13], cid[9];
	safe_input_str(sid, sizeof(sid), "学号：");
	safe_input_str(cid, sizeof(cid), "课程号：");
	
	if (g_iface.remove(g_ds, sid, cid) == 0)
		printf("[成功] 删除记录成功\n");
	else
		printf("[错误] 删除失败，未找到对应记录\n");
}

void upd_score() {
	char sid[13], cid[9];
	safe_input_str(sid, sizeof(sid), "学号：");
	safe_input_str(cid, sizeof(cid), "课程号：");
	int s = safe_input_int(0, 100, "新成绩（0-100）：");
	
	if (g_iface.update(g_ds, sid, cid, s) == 0)
		printf("[成功] 修改成绩成功\n");
	else
		printf("[错误] 修改失败，未找到对应记录\n");
}

void find_rec() {
	char sid[13], cid[9];
	safe_input_str(sid, sizeof(sid), "学号：");
	safe_input_str(cid, sizeof(cid), "课程号：");
	
	CourseRecord *r = g_iface.find(g_ds, sid, cid);
	if (r) {
		printf("\n");
		print_header();
		print_rec(*r);
	} else {
		printf("[错误] 未找到对应记录\n");
	}
}

void filter_menu() {
	if (g_iface.size(g_ds) == 0) {
		printf("[错误] 当前无数据，无法筛选\n");
		return;
	}
	
	FilterCondition cond;
	memset(&cond, 0, sizeof(cond));
	cond.score_min = cond.score_max = -1;
	char buf[100];
	
	printf("\n===== 多条件筛选 =====\n");
	safe_input_str(buf, sizeof(buf), "课程名（输入*表示不限）：");
	if (strcmp(buf, "*") != 0) {
		strcpy(cond.course_name, buf);
		cond.fuzzy_course = safe_input_int(0, 1, "是否模糊匹配？(1是 0否)：");
	}
	
	safe_input_str(buf, sizeof(buf), "学期（输入*表示不限）：");
	if (strcmp(buf, "*") != 0) strcpy(cond.semester, buf);
	
	safe_input_str(buf, sizeof(buf), "成绩下限（输入*表示不限）：");
	if (strcmp(buf, "*") != 0) cond.score_min = atoi(buf);
	
	safe_input_str(buf, sizeof(buf), "成绩上限（输入*表示不限）：");
	if (strcmp(buf, "*") != 0) cond.score_max = atoi(buf);
	
	safe_input_str(buf, sizeof(buf), "学院（输入*表示不限）：");
	if (strcmp(buf, "*") != 0) strcpy(cond.college, buf);
	
	int cnt = 0;
	CourseRecord *res = filter_records(g_ds, g_iface, cond, &cnt);
	printf("\n共找到 %d 条符合条件的记录\n", cnt);
	
	if (cnt > 0) {
		print_header();
		int show = cnt < 30 ? cnt : 30;
		for (int i = 0; i < show; i++) print_rec(res[i]);
		if (cnt > 30) printf("... 仅显示前30条，完整结果可导出查看\n");
		
		if (safe_confirm("是否导出筛选结果到CSV文件？(Y/N)：")) {
			char fn[256];
			safe_input_str(fn, sizeof(fn), "请输入导出文件名：");
			if (export_records_to_csv(res, cnt, fn) == 0)
				printf("[成功] 导出完成\n");
			else
				printf("[错误] 导出失败\n");
		}
	}
	free(res);
}

void sort_menu() {
	if (g_iface.size(g_ds) == 0) {
		printf("[错误] 当前无数据，无法排序\n");
		return;
	}
	
	SortRule rules[5];
	int n = 0;
	printf("\n===== 多关键字排序 =====\n");
	printf("可选字段：1-学号  2-姓名  3-成绩  4-学分  5-选课日期（输入0结束）\n");
	
	while (n < 5) {
		char prompt[64];
		sprintf(prompt, "第%d个排序字段：", n + 1);
		int f = safe_input_int(0, 5, prompt);
		if (f == 0) break;
		int d = safe_input_int(0, 1, "排序方向（0-升序 1-降序）：");
		rules[n].field = f - 1;
		rules[n].descending = d;
		n++;
	}
	
	if (n == 0) {
		printf("未设置排序规则，已取消\n");
		return;
	}
	
	int cnt = g_iface.size(g_ds);
	CourseRecord *recs = malloc(cnt * sizeof(CourseRecord));
	if (!recs) {
		printf("[错误] 内存不足\n");
		return;
	}
	
	CollectArg ca = {recs, 0};
	g_iface.traverse(g_ds, collect_cb, &ca);
	sort_records(recs, cnt, rules, n);
	
	printf("\n排序结果（前30条）：\n");
	print_header();
	int show = cnt < 30 ? cnt : 30;
	for (int i = 0; i < show; i++) print_rec(recs[i]);
	free(recs);
}

void stat_menu() {
	printf("\n===== 统计分析 =====\n");
	printf("1-课程选课人数  2-学生学分统计\n");
	printf("3-学院分布统计  4-成绩分布统计\n");
	int c = safe_input_int(1, 4, "请选择统计项：");
	
	switch(c) {
		case 1: stat_course_enrollment(g_ds, g_iface, 60); break;
		case 2: stat_student_credit(g_ds, g_iface); break;
		case 3: stat_college_distribution(g_ds, g_iface); break;
		case 4: stat_score_distribution(g_ds, g_iface); break;
	}
}

void batch_del() {
	int cnt = 0;
	g_iface.traverse(g_ds, count_expired_cb, &cnt);
	
	printf("\n===== 过期记录清理 =====\n");
	printf("基准日期：2026-09-01\n");
	if (cnt == 0) {
		printf("当前无过期记录，无需清理\n");
		return;
	}
	printf("即将删除选课日期早于 2023-09-01 的记录，共 %d 条\n", cnt);
	
	if (!safe_confirm("确认批量删除？此操作不可撤销 (Y/N)：")) {
		printf("已取消删除操作\n");
		return;
	}
	
	char (*keys)[21] = malloc(cnt * 21);
	if (!keys) {
		printf("[错误] 内存不足，操作失败\n");
		return;
	}
	
	ExpireArg ea = {keys, 0};
	g_iface.traverse(g_ds, collect_expire_cb, &ea);
	
	int del = 0;
	for (int i = 0; i < ea.idx; i++) {
		char sid[13], cid[9];
		strncpy(sid, keys[i], 12); sid[12] = 0;
		strncpy(cid, keys[i] + 12, 8); cid[8] = 0;
		if (g_iface.remove(g_ds, sid, cid) == 0) del++;
	}
	free(keys);
	printf("[成功] 已删除 %d 条过期记录\n", del);
}

void perf_test() {
	int n = safe_input_int(10, 100000, "请输入测试数据规模（10-100000）：");
	run_performance_test(n);
}

void save_file() {
	char fn[256];
	safe_input_str(fn, sizeof(fn), "请输入保存文件名：");
	
	if (save_to_csv(g_ds, g_iface, fn) == 0)
		printf("[成功] 数据保存完成\n");
	else
		printf("[错误] 保存失败，请检查路径权限\n");
}

int main() {
    // ========== 新增：设置控制台编码为 UTF-8，解决中文乱码 ==========
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);  // 设置输出代码页为 UTF-8
        SetConsoleCP(CP_UTF8);        // 设置输入代码页为 UTF-8
    #endif
    // ==============================================================

	init_ds(DS_AVL);
	int c;
	while (1) {
		show_menu();
		c = safe_input_int(0, 13, "请输入选项编号：");
		switch(c) {
		case 0:
			if (safe_confirm("确认退出程序？(Y/N)：")) {
				printf("程序已退出，再见\n");
				g_iface.destroy(g_ds);
				return 0;
			}
			break;
			case 1: switch_ds(); break;
			case 2: gen_data(); break;
			case 3: load_file(); break;
			case 4: add_rec(); break;
			case 5: del_rec(); break;
			case 6: upd_score(); break;
			case 7: find_rec(); break;
			case 8: filter_menu(); break;
			case 9: sort_menu(); break;
			case 10: stat_menu(); break;
			case 11: batch_del(); break;
			case 12: perf_test(); break;
			case 13: save_file(); break;
			default: printf("[错误] 无效选项\n");
		}
	}
}