#include "stats.h"

/* 4.1 课程选课人数与容量使用率 */
typedef struct { char id[9]; char name[50]; int cnt; } CourseStat;
typedef struct { CourseStat *data; int size; int cap; } CourseStatArr;

//按课程分组计数，计算容量使用率，格式化输出表格
static void course_cb(CourseRecord r, void *arg) {
	CourseStatArr *arr = (CourseStatArr*)arg;
	int found = -1;
	for (int i = 0; i < arr->size; i++) {
		if (strcmp(arr->data[i].id, r.course_id) == 0) { found = i; break; }
	}
	if (found >= 0) arr->data[found].cnt++;
	else {
		if (arr->size >= arr->cap) {
			arr->cap *= 2;
			arr->data = realloc(arr->data, arr->cap * sizeof(CourseStat));
		}
		strcpy(arr->data[arr->size].id, r.course_id);
		strcpy(arr->data[arr->size].name, r.course_name);
		arr->data[arr->size].cnt = 1;
		arr->size++;
	}
}


void stat_course_enrollment(void *ds, DSInterface iface, int capacity) {
	if (iface.size(ds) == 0) {
		printf("\n[错误] 当前无数据，无法统计\n");
		return;
	}
	
	CourseStatArr arr = {malloc(64*sizeof(CourseStat)), 0, 64};
	iface.traverse(ds, course_cb, &arr);
	
	printf("\n===== 课程选课人数与容量使用率 =====\n");
	printf("%-12s %-20s %-8s %-8s %-10s\n", "课程编号", "课程名称", "人数", "容量", "使用率");
	printf("------------------------------------------------------------\n");
	for (int i = 0; i < arr.size; i++) {
		printf("%-12s %-20s %-8d %-8d %.1f%%\n",
			   arr.data[i].id, arr.data[i].name, arr.data[i].cnt,
			   capacity, 100.0f * arr.data[i].cnt / capacity);
	}
	printf("共 %d 门课程\n", arr.size);
	free(arr.data);
}

/* 4.2 学生选课门数与总学分 */
typedef struct { char sid[13]; char name[20]; int cnt; float credit; } StuStat;
typedef struct { StuStat *data; int size; int cap; } StuStatArr;

//按学生分组，累计选课门数与总学分，输出前 20 条
static void stu_cb(CourseRecord r, void *arg) {
	StuStatArr *arr = (StuStatArr*)arg;
	int found = -1;
	for (int i = 0; i < arr->size; i++) {
		if (strcmp(arr->data[i].sid, r.student_id) == 0) { found = i; break; }
	}
	if (found >= 0) { arr->data[found].cnt++; arr->data[found].credit += r.credit; }
	else {
		if (arr->size >= arr->cap) {
			arr->cap *= 2;
			arr->data = realloc(arr->data, arr->cap * sizeof(StuStat));
		}
		strcpy(arr->data[arr->size].sid, r.student_id);
		strcpy(arr->data[arr->size].name, r.name);
		arr->data[arr->size].cnt = 1;
		arr->data[arr->size].credit = r.credit;
		arr->size++;
	}
}

void stat_student_credit(void *ds, DSInterface iface) {
	if (iface.size(ds) == 0) {
		printf("\n[错误] 当前无数据，无法统计\n");
		return;
	}
	
	StuStatArr arr = {malloc(128*sizeof(StuStat)), 0, 128};
	iface.traverse(ds, stu_cb, &arr);
	
	printf("\n===== 学生选课统计（前20条） =====\n");
	printf("%-14s %-10s %-8s %-8s\n", "学号", "姓名", "门数", "总学分");
	printf("--------------------------------------------------------\n");
	int show = arr.size < 20 ? arr.size : 20;
	for (int i = 0; i < show; i++)
		printf("%-14s %-10s %-8d %.1f\n", arr.data[i].sid, arr.data[i].name, arr.data[i].cnt, arr.data[i].credit);
	printf("共 %d 名学生\n", arr.size);
	free(arr.data);
}

/* 4.3 学院选课分布 */
typedef struct { char name[50]; int cnt; } CollegeStat;
typedef struct { CollegeStat *data; int size; int cap; } CollegeStatArr;

//按学院名称分组，统计各学院选课人次
static void college_cb(CourseRecord r, void *arg) {
	CollegeStatArr *arr = (CollegeStatArr*)arg;
	int found = -1;
	for (int i = 0; i < arr->size; i++) {
		if (strcmp(arr->data[i].name, r.college) == 0) { found = i; break; }
	}
	if (found >= 0) arr->data[found].cnt++;
	else {
		if (arr->size >= arr->cap) {
			arr->cap *= 2;
			arr->data = realloc(arr->data, arr->cap * sizeof(CollegeStat));
		}
		strcpy(arr->data[arr->size].name, r.college);
		arr->data[arr->size].cnt = 1;
		arr->size++;
	}
}

void stat_college_distribution(void *ds, DSInterface iface) {
	if (iface.size(ds) == 0) {
		printf("\n[错误] 当前无数据，无法统计\n");
		return;
	}
	
	CollegeStatArr arr = {malloc(16*sizeof(CollegeStat)), 0, 16};
	iface.traverse(ds, college_cb, &arr);
	
	printf("\n===== 学院选课人次分布 =====\n");
	printf("%-25s %-8s\n", "学院", "人次");
	printf("--------------------------------\n");
	for (int i = 0; i < arr.size; i++)
		printf("%-25s %-8d\n", arr.data[i].name, arr.data[i].cnt);
	free(arr.data);
}

/* 4.5 成绩分布 */
typedef struct { int excellent, good, medium, pass, fail; } ScoreStat;

//按优秀 / 良好 / 中等 / 及格 / 不及格五个分数段累加计数，计算占比
static void score_cb(CourseRecord r, void *arg) {
	ScoreStat *s = (ScoreStat*)arg;
	if (r.score >= 90) s->excellent++;
	else if (r.score >= 80) s->good++;
	else if (r.score >= 70) s->medium++;
	else if (r.score >= 60) s->pass++;
	else s->fail++;
}

void stat_score_distribution(void *ds, DSInterface iface) {
	if (iface.size(ds) == 0) {
		printf("\n[错误] 当前无数据，无法统计\n");
		return;
	}
	
	ScoreStat s = {0,0,0,0,0};
	iface.traverse(ds, score_cb, &s);
	int total = s.excellent + s.good + s.medium + s.pass + s.fail;
	
	printf("\n===== 成绩分布统计 =====\n");
	printf("%-8s %-8s %-8s\n", "等级", "人数", "占比");
	printf("------------------------\n");
	printf("%-8s %-8d %.1f%%\n", "优秀", s.excellent, total?100.0*s.excellent/total:0);
	printf("%-8s %-8d %.1f%%\n", "良好", s.good, total?100.0*s.good/total:0);
	printf("%-8s %-8d %.1f%%\n", "中等", s.medium, total?100.0*s.medium/total:0);
	printf("%-8s %-8d %.1f%%\n", "及格", s.pass, total?100.0*s.pass/total:0);
	printf("%-8s %-8d %.1f%%\n", "不及格", s.fail, total?100.0*s.fail/total:0);
	printf("总计: %d\n", total);
}
