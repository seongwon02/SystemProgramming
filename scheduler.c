#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "util.h"
#include "scheduler.h"
#include "date_check.h"

time_t timer;
struct tm* t;

void initTime(Time* x) {
    timer = time(NULL);
    t = localtime(&timer);

    x->year = t->tm_year + 1900;
    x->month = t->tm_mon + 1;
    x->day = t->tm_mday;
}


//Interval Calculation
void calInterval(Event* event_t) {
    event_t->interval = 0;
    int year = event_t->date_start.year;
    int month = event_t->date_start.month;
    int day = event_t->date_start.day;

    int endYear = event_t->date_end.year;
    int endMonth = event_t->date_end.month;
    int endDay = event_t->date_end.day;

    while (year < endYear || (year == endYear && month < endMonth) || (year == endYear && month == endMonth && day < endDay)) {
        event_t->interval++;
        day++;
        if (day > daysInMonth(month, year)) {
            day = 1;
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
    }
    event_t->interval++;
}

//Dday Calculation

void calDday(Event* event_t, Time current) {
    event_t->Dday = 0;
    int year = current.year;
    int month = current.month;
    int day = current.day;

    int endYear = event_t->date_end.year;
    int endMonth = event_t->date_end.month;
    int endDay = event_t->date_end.day;

    if (year > endYear || (year == endYear && month > endMonth) || (year == endYear && month == endMonth && day > endDay)) {
        event_t->Dday = -1;
        return;
    }
    while (year < endYear || (year == endYear && month < endMonth) || (year == endYear && month == endMonth && day < endDay)) {
        event_t->Dday++;
        day++;
        if (day > daysInMonth(month, year)) {
            day = 1;
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
    }
}


//Weigth Calcuation
void calWeight(Event* event_t, int importance) {
    const double a = 100.0;
    const double b = 1.0;

    if (event_t->Dday == 0)
        event_t->weight = importance;
    else
        event_t->weight = a * (1.0 / event_t->Dday) + b * importance;
}

//Thread: Updaye Dday and Weight By DayChange
void updateDdayAndWeights(Event events[], int count) {
    Time current;
    initTime(&current);

    for(int i = 0; i < count; i++) {
        calDday(&events[i], current);
        if (events[i].importance != -1) { // 일반 일정인 경우 weight 계산 안함
            calWeight(&events[i], events[i].importance);
        }
    }
}

void add_schedule() {
    if (event_count >= MAX_EVENTS) {
        popup_message("Event list is full. Cannot add more schedules.");
        return;
    }

    Time current;
    initTime(&current);

    // 입력 필드 정의
    char title[50] = {0}, details[100] = {0};
    char start_date[20] = {0}, end_date[20] = {0};
    char start_time[10] = {0}, end_time[10] = {0};
    char importance_buffer[10] = {0}, quantity_buffer[10] = {0}, reminder_buffer[5] = {0};

    InputField fields[] = {
        {"Enter schedule title", title, sizeof(title), validate_title, 0},
        {"Enter start date (YYYY MM DD)", start_date, sizeof(start_date), validate_date_wrapper, 0},
        {"Enter start time (HH MM)", start_time, sizeof(start_time), validate_time_wrapper, 1}, // 공백 허용
        {"Enter end date (YYYY MM DD)", end_date, sizeof(end_date), validate_date_wrapper, 0},
        {"Enter end time (HH MM)", end_time, sizeof(end_time), validate_time_wrapper, 1}, // 공백 허용
        {"Enter importance (0-5)", importance_buffer, sizeof(importance_buffer), validate_importance, 0},
        {"Enter quantity (integer)", quantity_buffer, sizeof(quantity_buffer), validate_quantity, 0},
        {"Set reminder (1: Yes, 0: No)", reminder_buffer, sizeof(reminder_buffer), validate_reminder, 0},
        {"Enter details", details, sizeof(details), NULL, 1} // 공백 허용
    };

    UIScreen screen = {
        "Add Schedule",
        fields,
        sizeof(fields) / sizeof(fields[0])
    };

    // 입력 화면 초기화
    active_screen = &screen;
    current_step = 0;

    // 사용자 입력 처리
    if (process_user_input(&screen) != 0) {
        popup_message("Schedule addition canceled.");
        active_screen = NULL;
        return;
    }

    // 데이터 파싱 및 유효성 검사
    Event new_event;
    int year_start, month_start, day_start, year_end, month_end, day_end;
    int hour_start, minute_start, hour_end, minute_end;
    double importance, quantity;
    int reminder;

    sscanf(start_date, "%d %d %d", &year_start, &month_start, &day_start);
    sscanf(start_time, "%d %d", &hour_start, &minute_start);
    sscanf(end_date, "%d %d %d", &year_end, &month_end, &day_end);
    sscanf(end_time, "%d %d", &hour_end, &minute_end);
    importance = atof(importance_buffer);
    quantity = atof(quantity_buffer);
    reminder = atoi(reminder_buffer);

    // 스케줄 데이터 저장
    new_event.id = ++last_event_id;
    strncpy(new_event.title, title, sizeof(new_event.title));

    new_event.date_start.year = year_start;
    new_event.date_start.month = month_start;
    new_event.date_start.day = day_start;
    new_event.date_start.hour = hour_start; // 기본값
    new_event.date_start.minute = minute_start;

    new_event.date_end.year = year_end;
    new_event.date_end.month = month_end;
    new_event.date_end.day = day_end;
    new_event.date_end.hour = hour_end; // 기본값
    new_event.date_end.minute = minute_end;

    new_event.importance = importance;
    new_event.reminder = reminder;
    strncpy(new_event.details, details, sizeof(new_event.details));
    calInterval(&new_event);
    calDday(&new_event, current);
    calWeight(&new_event, importance);
    new_event.quantity = quantity / new_event.interval;

    // 배열에 새 스케줄 추가
    events[event_count++] = new_event;

    // 중요도에 따라 정렬
    sortTodo(events, event_count);

    popup_message("Schedule successfully added!");

    // 화면 초기화
    active_screen = NULL;
}
