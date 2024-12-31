#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct Time {
	int year;
	int month;
	int day;
}Time;

void add_schedule(); // 일정 오토 스케줄링 기능

void initTime(Time* x); //current Time
void calInterval(Event* event_t);
void calDday(Event* event_t, Time current);
void calWeight(Event* event_t, int importance);
void updateDdayAndWeights(Event events[], int count);

#endif
