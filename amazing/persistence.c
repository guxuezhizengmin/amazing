#include "persistence.h"
/*功能：遍历回调函数，将单条记录格式化写入 CSV 文件。*/
static void save_cb(CourseRecord record, void *arg) {
	FILE *fp = (FILE*)arg;
	fprintf(fp, "%s,%s,%s,%s,%s,%.1f,%s,%s,%d\n",
			record.student_id, record.name, record.college,
			record.course_id, record.course_name, record.credit,
			record.semester, record.enroll_date, record.score);
}
/*功能：将当前数据结构中的全部记录保存为 CSV 文件
逻辑：打开文件写入表头，通过遍历回调逐条写入记录，字段顺序与结构体一一对应*/
int save_to_csv(void *ds, DSInterface iface, const char *filename) {
	FILE *fp = fopen(filename, "w");
	if (!fp) return -1;
	fprintf(fp, "学号,姓名,学院,课程编号,课程名称,学分,选课学期,选课日期,成绩\n");
	iface.traverse(ds, save_cb, fp);
	fclose(fp);
	return 0;
}
/*功能：从 CSV 文件加载记录到数据结构中
逻辑：逐行读取文件，用 strtok 按逗号分割字段，
解析后调用插入接口存入结构，自动跳过表头行。*/
int load_from_csv(void *ds, DSInterface iface, const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) return -1;
	
	char line[512];
	fgets(line, sizeof(line), fp); /* 跳过表头 */
	
	int count = 0;
	while (fgets(line, sizeof(line), fp)) {
		CourseRecord r;
		memset(&r, 0, sizeof(r));
		
		char *token = strtok(line, ",");
		if (!token) continue; 
		strncpy(r.student_id, token, 12);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		strncpy(r.name, token, 19);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		strncpy(r.college, token, 49);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		strncpy(r.course_id, token, 8);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		strncpy(r.course_name, token, 49);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		r.credit = atof(token);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		strncpy(r.semester, token, 7);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		strncpy(r.enroll_date, token, 10);
		
		token = strtok(NULL, ",");
		if (!token) continue; 
		r.score = atoi(token);
		
		if (iface.insert(ds, r) == 0) count++;
	}
	fclose(fp);
	return count;
}
