#include <stdio.h>
#include <curses.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "calendar_control.h"
#include "calendar_display.h"
#include "calendar.h"
#include "../date_check.h"


int year = 0, month = 0, day = 0;
int td_year = 0, td_month = 0, td_day = 0;
int color = 0, prev_first_color = 0, cur_first_color = 0;

int draw_calendar()
{
    stopDateMonitor();

    int ch;
	// 현재 날짜와 시간을 가져옴
    time_t t = time(NULL);
    struct tm *date = localtime(&t);   
    
    td_year = date->tm_year + 1900;  // 현재 연도 (tm_year는 1900년부터 시작)
    td_month = date->tm_mon + 1;     // 현재 월 (tm_mon은 0부터 시작)
	td_day = date->tm_mday;          // 현재 일

    year = td_year;
    month = td_month;
    day = td_day;
	
	clear();
	start_color();
    
    // 글자 색상
	init_pair(1, COLOR_RED, COLOR_BLACK);   // 일요일 기본
    init_pair(2, COLOR_BLUE, COLOR_BLACK);  // 토요일 기본
    
    init_pair(3, COLOR_BLACK, COLOR_WHITE); // 평일 반전
    init_pair(4, COLOR_RED, COLOR_WHITE);   // 일요일 반전
    init_pair(5, COLOR_BLUE, COLOR_WHITE);  // 토요일 반전

    init_pair(6, COLOR_WHITE, COLOR_RED);   // 초과 일정 수 
    init_pair(7, COLOR_WHITE, COLOR_MAGENTA);   // 막대 일정 1
    init_pair(8, COLOR_WHITE, COLOR_GREEN);   // 막대 일정 2
    init_pair(9, COLOR_WHITE, COLOR_CYAN);   // 막대 일정 3
    
    nodelay(stdscr, TRUE); // getch()를 비차단 모드로 설정
    keypad(stdscr, TRUE);   // 특수키 허용(화살표 얻기 위함)
    
    // 초기 화면 출력시 월간 캘린더 출력
    show_calendar();
    
    // 다음 동작 입력(q일 때 종료)
    while(1)
    {
        // 동작 설명 및 다음 실행할 동작 입력
        mvprintw(LINES - 2, 0, "b: back / m: monthly / w: weekly / d: daily / ->, <-: change date ");
        move(LINES - 1, 0);
        refresh();
        ch = getch();

        switch(ch)
        {
            case('b'): case('B'): // 종료   
                endwin();
                return 0;
            case('m'): case('M'): // 달 기준 캘린더
                state = 1;
                show_calendar();
                break;
            case('w'): case('W'): // 주 기준 캘린더
                state = 2;
                show_calendar();
                break;
            case('d'): case('D'): // 일 기준 캘린더
                state = 3;
                show_calendar();
                break;
            case(KEY_RIGHT): // 다음 날짜 이동
                color = ((color + 2) % 3);
                prev_first_color = cur_first_color;
                cur_first_color = color;
                change_date(date, 1);
                break;
            case(KEY_LEFT): // 이전 날짜 이동
                color = prev_first_color;
                cur_first_color = color;
                change_date(date, -1);
                break;
            case(ERR):  // 비차단 모드에서 입력 상태 유지
                usleep(100000); // 0.1초 대기
                break;
            default:
                break;
        }    
    }

    //initializeDateMonitor();
    startDateMonitor();
	return 0;
}
