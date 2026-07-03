#ifndef FILTER_SORT_H
#define FILTER_SORT_H

#include "common.h"

typedef struct {
	char course_name[50];
	int  fuzzy_course;
	char semester[8];
	int  score_min;
	int  score_max;
	char college[50];
} FilterCondition;

typedef enum {
	SORT_STUDENT_ID, SORT_NAME, SORT_SCORE, SORT_CREDIT, SORT_ENROLL_DATE
} SortField;

typedef struct {
	SortField field;
	int descending;
} SortRule;

CourseRecord* filter_records(void *ds, DSInterface iface, FilterCondition cond, int *out_count);
void sort_records(CourseRecord *records, int count, SortRule *rules, int rule_count);
int export_records_to_csv(CourseRecord *records, int count, const char *filename);

#endif
