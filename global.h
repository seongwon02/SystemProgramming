#ifndef GLOBAL_H
#define GLOBAL_H

#define MAX_EVENTS 100
#define MAX_HABITS 100
#define EVENT_FILE "event.txt"
#define HABIT_FILE "habit.txt"

// 화면 상수
#define MAIN_SCREEN 0
#define EVENT_SCREEN 1
#define HABIT_SCREEN 2
#define CALENDAR_SCREEN 3
#define DEFAULT_SCREEN 4

// 구조체 정의
typedef struct {
    int id;
    char title[50];
    struct {
        int year, month, day, hour, minute;
    } date_start, date_end;
    int importance;     // 0 ~ 5 사이 정수 (일반 일정은 default : -1)
    double quantity;    // 하루 당 분량
    double interval;    // 마감일 - 시작일
    double Dday;        // 마감일 - 현재일(now)
    double weight;      // 가중치 식에 따라 계산 (지역변수 importance를 사용해서 계산)
    int reminder;       // 리마인더
    char details[100];

    /* default 정리 (일반 일정에 필요 없는 값)  >> 다 기본값 -1
        quantity, importance, interval, weight
    */
} Event;

typedef struct {
    int id;
    char name[50];
    int streak;
    int is_done;    // 유지 일수
} Habit;

// 전역 변수 선언
extern Event events[MAX_EVENTS];
extern int event_count;
extern int last_event_id;

extern Habit habits[MAX_HABITS];
extern int habit_count;
extern int last_habit_id;

extern int current_screen;
extern char last_checked_date[11];

#endif

// typedef struct {
//     int id;
//     char title[50];
//     Time date_start;
//     Time date_end;
//     int importance;     // 0 ~ 5 사이 정수
//     double quantity;    // 총 분량 / 총 일수 = 하루당 분량
    
//     // 아래는 계산되는 값
//     double interval;    // 마감일 - 시작일
//     double Dday;        // 마감일 - 현재일(now)
//     double weight;      // 가중치 식에 따라 계산 (지역변수 importance를 사용해서 계산) >> default : -1

//     char details[100];  // 상세 기록

//     /* 계산 되는 값
//     interval
//     Dday
//     weight
//     */

//     /* default 정리 (일반 일정에 필요 없는 값)  >> 다 기본값 -1
//     quantity
//     importance
//     interval
//     weight
//     */

//    /* 저장 필드 값 변경 사항 기록 */
//    /*
//    기존 : id|title|date_start|date_end|interval|Dday|weight|quantity|details
//    변경 : id|title|date_start|date_end|importance|quantity|interval|Dday|weight|details
//    */
// } Event;
