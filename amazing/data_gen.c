#include "data_gen.h"
#include <math.h>

/* 严格遵循任务书附录数据生成规则 */
static const char *surnames = "赵钱孙李周吴郑王冯陈褚卫蒋沈韩杨朱秦尤许何吕施张孔曹严华金魏陶姜";
static const char *names_chars = "伟芳娜敏静丽强磊军洋勇艳杰娟涛明超秀兰霞平刚桂英";

static const char *colleges[] = {
	"计算机科学与工程学院", "数学与统计学院", "电子信息工程学院",
	"机械工程学院", "经济管理学院", "外国语学院",
	"土木建筑学院", "物理与光电工程学院"
};
static const int college_total = 8;

static const char *course_ids[] = {
	"CS300102", "CS300201", "CS300305", "CS300408",
	"MA200103", "MA200206", "EE400101", "EE400204",
	"ME500102", "ME500205", "EC600103", "EC600207"
};
static const char *course_names[] = {
	"数据结构与算法", "操作系统原理", "数据库系统原理", "计算机网络",
	"高等数学", "线性代数", "模拟电子技术", "数字电路",
	"机械设计基础", "工程力学", "微观经济学", "管理学原理"
};
static const float credits[] = {3.5, 3.0, 3.5, 3.0, 4.0, 3.0, 3.0, 2.5, 3.5, 3.0, 2.0, 2.5};
static const int course_total = 12;

/* Box-Muller 正态分布随机数 */
/*Box-Muller 算法生成正态分布随机数，用于模拟真实成绩分布（均值 75，标准差 12）*/
static double rand_normal(double mean, double std) {
	static double spare;
	static int has_spare = 0;
	if (has_spare) { has_spare = 0; return mean + std * spare; }
	
	double u1, u2, s;
	do {
		u1 = rand() / (RAND_MAX + 1.0);
		u2 = rand() / (RAND_MAX + 1.0);
		s = u1*u1 + u2*u2;
	} while (s >= 1 || s == 0);
	
	s = sqrt(-2.0 * log(s) / s);
	spare = u2 * s;
	has_spare = 1;
	return mean + std * u1 * s;
}
/*按规则生成 12 位学号：4 位入学年份 + 2 位学院码 + 6 位序号*/
static void gen_student_id(int index, char *buf) {
	int year = 2020 + (index % 7);
	int college_code = 1 + (index % 20);
	int seq = index % 1000000;
	sprintf(buf, "%04d%02d%06d", year, college_code, seq);
}
/*从姓氏库与名字库中随机选取，生成符合中文习惯的姓名*/
static void gen_name(char *buf) {
	int s_idx = rand() % (strlen(surnames)/3);
	strncpy(buf, surnames + s_idx*3, 3);
	buf[3] = '\0';
	
	int name_len = 1 + rand() % 2;
	for (int i = 0; i < name_len; i++) {
		int n_idx = rand() % (strlen(names_chars)/3);
		strncat(buf, names_chars + n_idx*3, 3);
	}
}
/*根据学年与学期生成合理选课日期：秋季学期 9 月、春季学期 2 月*/
static void gen_date(int year, int semester, char *buf) {
	int month, day;
	if (semester == 2) { month = 9 + rand()%2; day = 1 + rand()%30; }
	else { month = 2 + rand()%2; day = 1 + rand()%28; }
	sprintf(buf, "%04d-%02d-%02d", year, month, day);
}

CourseRecord* generate_records(int count) {
	if (count <= 0) return NULL;
	CourseRecord *records = (CourseRecord*)malloc(count * sizeof(CourseRecord));
	if (!records) return NULL;
	
	srand((unsigned int)time(NULL));
	for (int i = 0; i < count; i++) {
		CourseRecord r;
		memset(&r, 0, sizeof(r));
		
		gen_student_id(i, r.student_id);
		gen_name(r.name);
		strcpy(r.college, colleges[rand() % college_total]);
		
		int ci = rand() % course_total;
		strcpy(r.course_id, course_ids[ci]);
		strcpy(r.course_name, course_names[ci]);
		r.credit = credits[ci];
		
		int year = 2020 + rand() % 7;
		int sem = 1 + rand() % 2;
		sprintf(r.semester, "%04d-%02d", year, sem);
		gen_date(year, sem, r.enroll_date);
		
		int score = (int)rand_normal(75, 12);
		r.score = score < 0 ? 0 : (score > 100 ? 100 : score);
		
		records[i] = r;
	}
	return records;
}
