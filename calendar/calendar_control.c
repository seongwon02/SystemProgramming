#include <stdio.h>
#include <curses.h>
#include <sys/ioctl.h>
#include <time.h>
#include <stdlib.h>
#include "calendar_control.h"
#include "calendar_display.h"

struct winsize wbuf;
int state = MONTHLY_CALENDAR;

// 캘린더를 보여주는 함수
void show_calendar()
{
	clear();
    // 터미널 화면정보 다시 가져오기
    if(ioctl(0, TIOCGWINSZ, &wbuf) != -1) {     
        switch (state)
        {
        // row 20 이상, col 80 이상 시 월별 기준 달력 표시를 달리함
        case (MONTHLY_CALENDAR):   
            if (wbuf.ws_row >= 20 && wbuf.ws_col >= 80) 
                show_big_month(year, month, day, &wbuf);            
            else 
                show_small_month(year, month, &wbuf);
            break;
        case (WEEKLY_CALENDAR):
            show_week(year, month, day, &wbuf);
            break;
        case (DAILY_CALENDAR):
            show_day(year, month, day, &wbuf);
            break;
        default:
            break;
        }
    }
    // 터미널 화면 정보 가져오기 실패
    else
    {
        fprintf(stderr, "Could not import calendar.\n");
        exit(-1);
    }
}

// 날짜 변경하는 함수
void change_date(struct tm *date, int num)
{
    if (state == MONTHLY_CALENDAR) // 월간 캘린더인 경우 다음 달 또는 저번 달로 이동 
        date->tm_mon += num * 1;
    else if (state == WEEKLY_CALENDAR)    // 주간 캘린더인 경우 다음 주 또는 저번 달로 이동 
        date->tm_mday += num * 7;
    else if (state == DAILY_CALENDAR)    // 일간 캘린더인 경우 내일 또는 어제로 이동 
        date->tm_mday += num * 1;
    
    // 날짜 정보 재설정 후 화면 출력
    mktime(date);

    year = date->tm_year + 1900;  // 현재 연도 (tm_year는 1900년부터 시작)
    month = date->tm_mon + 1;     // 현재 월 (tm_mon은 0부터 시작)
	day = date->tm_mday;          // 현재 일
    
    show_calendar();
}
