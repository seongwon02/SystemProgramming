#ifndef CALENDAR_H
#define CALENDAR_H

#define MONTHLY_CALENDAR 1
#define WEEKLY_CALENDAR 2
#define DAILY_CALENDAR 3

extern int year, month, day;	// 날짜 계산 시 사용
extern int td_year, td_month, td_day;	// 오늘의 날짜 저장
extern int state; // 화면 출력 상태(1: 큰 월간 캘린더, 2: 작은 월간 캘린더, 3: 주간 캘린더, 4: 일간 캘린더)
extern int color; // 연속 일정의 막대 일정 색 표현
extern int prev_first_color; // 이전 달의 쓰이는 막대 일정의 처음 색 저장
extern int cur_first_color; // 이전 달의 쓰이는 막대 일정의 처음 색 저장

int draw_calendar(); 

#endif
