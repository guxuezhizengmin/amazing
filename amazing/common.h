#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 选课记录结构体，严格对应任务书字段 */
typedef struct {
	char student_id[13];   /* 12位学号 */
	char name[20];         /* 学生姓名 */
	char college[50];      /* 所属学院 */
	char course_id[9];     /* 8位课程编号 */
	char course_name[50];  /* 课程名称 */
	float credit;          /* 课程学分 */
	char semester[8];      /* 选课学期 YYYY-XX */
	char enroll_date[11];  /* 选课日期 YYYY-MM-DD */
	int score;             /* 成绩 0-100 */
} CourseRecord;

/* 通用数据结构操作接口（业务与存储解耦） */
typedef struct {
	int  (*insert)  (void *ds, CourseRecord record);
	int  (*remove)  (void *ds, const char *student_id, const char *course_id);
	int  (*update)  (void *ds, const char *student_id, const char *course_id, int new_score);
	CourseRecord* (*find)(void *ds, const char *student_id, const char *course_id);
	void (*traverse)(void *ds, void (*callback)(CourseRecord, void*), void *arg);
	int  (*size)    (void *ds);
	void (*destroy) (void *ds);
} DSInterface;

/* 基础工具函数 */
int  compare_date(const char *date1, const char *date2);
int  fuzzy_match(const char *str, const char *substr);
void make_key(const char *student_id, const char *course_id, char *key_buf);

/* 鲁棒性增强：安全输入工具 */
int  safe_input_int(int min, int max, const char *prompt);
void safe_input_str(char *buf, int max_len, const char *prompt);
int  safe_confirm(const char *prompt);

#endif
