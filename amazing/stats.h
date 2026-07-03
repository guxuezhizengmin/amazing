#ifndef STATS_H
#define STATS_H

#include "common.h"

void stat_course_enrollment(void *ds, DSInterface iface, int capacity);
void stat_student_credit(void *ds, DSInterface iface);
void stat_college_distribution(void *ds, DSInterface iface);
void stat_score_distribution(void *ds, DSInterface iface);

#endif
