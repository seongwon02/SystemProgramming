#ifndef CALENDAR_GET_EVENT_H
#define CALENDAR_GET_EVENT_H

#define EVENT_FILE "event.txt"

typedef struct info info;

// 각 월의 일수를 저장한 배열 (윤년이 아닌 경우)
extern int days_in_month[];

// 윤년인지 확인하는 함수
int is_leap_year(int year);
// 연도와 월을 입력받아 해당 월의 첫 번째 요일을 계산하는 함수
int get_start_day_of_month(int year, int month);

// 달 기준 캘린더에 연속적인 일정을 가져와 출력하는 함수
void get_continuous_event(int year, int month, int start_wday, int width, int x_coordinate[6][7], int y_coordinate[6][7], int used_coordinate[6][7][3], int event_cnt[32]);
// 달 기준 캘린더에 단일 일정을 가져와 출력하는 함수
void get_single_event(int year, int month, int start_wday, int width, int x_coordinate[6][7], int y_coordinate[6][7],  int used_coordinate[6][7][3], int event_cnt[32]);

//주 기준 캘린더에서 일정을 가져와 출력하는 함수
void get_event_week(int year, int month, int start_day, int end_day, int width, int x, int y_coordinate[7], int used_coordinate[][7]);

// 표 형식의 캘린더에 일정을 가져와 출력하는 함수
void get_event_in_table(int year, int month, int day, int width, int x, int y_coordinate[]);

#endif
