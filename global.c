// global.c
#include "global.h"

Event events[MAX_EVENTS] = {0};  // events 배열 초기화
int event_count = 0;             // 이벤트 개수 초기화
int last_event_id = 0;           // 마지막 ID 초기화

Habit habits[MAX_HABITS] = {0};  // habits 배열 초기화  
int habit_count = 0;             // 습관 개수 초기화
int last_habit_id = 0;           // 마지막 ID 초기화
char last_checked_date[11] = {0};  // 마지막 체크 날짜 초기화

int current_screen = MAIN_SCREEN;  // 추가: 초기 화면을 메인으로 설정