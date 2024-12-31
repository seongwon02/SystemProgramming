#include <ncurses.h>
#include <string.h>
#include <time.h>
#include "display.h"
#include "global.h"
#include "event.h"
#include "habit.h"

// 리마인더 리스트 출력
static void draw_reminders(int start_y, int column_width) {
    mvprintw(start_y++, 5, "Reminders:");
    for (int i = 0; i < event_count; i++) {
        if (events[i].reminder == 1 && events[i].Dday <= 3) {  
            if (events[i].Dday == 0) {
                mvprintw(start_y++, 5, "- %s (D-day)", events[i].title);
            } else if (events[i].Dday >= 0) {
                mvprintw(start_y++, 5, "- %s (D-%.0f)", events[i].title, events[i].Dday);
            }
        }
    }
}

// 습관 리스트 출력
static void draw_habits(int start_y, int column_width) {
    mvprintw(start_y++, column_width + 5, "Habits:");
    for (int i = 0; i < habit_count; i++) {
        if(habits[i].is_done!=0){
            mvprintw(start_y++, column_width + 5, "- %s is done(Streak: %d days)", habits[i].name, habits[i].streak);
        }
        else{
            mvprintw(start_y++, column_width + 5, "- %s isn't done(Streak: %d days)", habits[i].name, habits[i].streak);
        }
        
    }
}

// 가로 레이아웃 메뉴 출력
static void draw_horizontal_menu() {
    int height, width;
    getmaxyx(stdscr, height, width);

	(void)height;  // 높이를 명시적으로 무시
    
	const char *menu_items[] = {"1. Calendar", "2. Event Management",
                                "3. Habit Management", "q. Quit"};
    int menu_count = sizeof(menu_items) / sizeof(menu_items[0]);
    int menu_y = 10;
    int spacing = width / (menu_count + 1);

    // 가로 메뉴 출력
    for (int i = 0; i < menu_count; i++) {
        mvprintw(menu_y, spacing * (i + 1) - strlen(menu_items[i]) / 2, "%s", menu_items[i]);
    }

    refresh();
}

// 리스트 레이아웃 메뉴 출력
static void draw_vertical_menu() {
    // 세로 메뉴 출력
    mvprintw(5, 5, "1. Calendar");
    mvprintw(6, 5, "2. Event Management");
    mvprintw(7, 5, "3. Habit Management");
    mvprintw(8, 5, "q. Quit");
    refresh();
}

// 메인 메뉴 출력
void draw_main_menu() {
    int height, width;
    getmaxyx(stdscr, height, width);

    (void)height;  // 높이를 명시적으로 무시

	if (width >= 80) {
        draw_horizontal_menu();  // 가로 메뉴
    } else {
        draw_vertical_menu();    // 세로 메뉴
    }
}

// 타이틀 출력
void draw_title() {
    char title[50]; // 타이틀(월, 일, 년)
    char months_to_string[][20] = {"Januray", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    time_t t = time(NULL);
    struct tm *date = localtime(&t);   
    
    int y = date->tm_year + 1900; // 현재 연도 (tm_year는 1900년부터 시작)
    int m = date->tm_mon + 1;     // 현재 월 (tm_mon은 0부터 시작)
    int d = date->tm_mday;        // 현재 일
    snprintf(title, sizeof(title), "%s %d, %d", months_to_string[m-1], d, y);
    
    // 타이틀이 출력될 위치
    int title_row = 3;
    int title_col = (COLS - strlen(title)) / 2;

    attron(A_BOLD);
    mvprintw(title_row, title_col, "%s", title);
    attroff(A_BOLD);
}

// 리마인더와 습관 리스트 출력
void draw_lists() {
    int height, width;
    getmaxyx(stdscr, height, width);

	(void)height;  // 높이를 명시적으로 무시

    if (width >= 80) {
        // 가로 레이아웃
        int list_start_y = 15;
        int column_width = (width - 12) / 2;

        draw_reminders(list_start_y, column_width); // 리마인더 출력
        draw_habits(list_start_y, column_width);    // 습관 출력
    } else {
        // 세로 레이아웃
        int list_start_y = 12;

        draw_reminders(list_start_y, 0);  // 리마인더 출력
        draw_habits(list_start_y + 6, 0); // 습관 출력 (리마인더 아래에 배치)
    }

    refresh();
}

// 일정 관리 서브메뉴 화면
void draw_event_screen() {
    clear();
    int height, width;
    getmaxyx(stdscr, height, width);

    mvprintw(height / 2 - 4, (width - 25) / 2, "1. Add Event");
    mvprintw(height / 2 - 2, (width - 25) / 2, "2. Modify Event");
   	mvprintw(height / 2, (width - 25) / 2, "3. Delete Event");
    mvprintw(height / 2 + 2, (width - 25) / 2, "4. Auto Scheduling");
    mvprintw(height / 2 + 4, (width - 25) / 2, "5. Back to Main Menu");
    
	refresh();
}

// 습관 관리 서브메뉴 화면
void draw_habit_screen() {
    clear();
    int height, width;
    getmaxyx(stdscr, height, width);

	mvprintw(height / 2 - 4, (width - 30) / 2, "1. Add Habit");
    mvprintw(height / 2 - 2, (width - 30) / 2, "2. Change Habit");
    mvprintw(height / 2, (width - 30) / 2, "3. Delete Habit");
    mvprintw(height / 2 + 2, (width - 30) / 2, "4. Mark Habit Success");
    mvprintw(height / 2 + 4, (width - 30) / 2, "5. Back to Main Menu");
    
	refresh();
}

// 서브-서브 화면 출력
void draw_ui_screen(const UIScreen *screen, int current_step) {
    clear();

    mvprintw(2, 10, "%s", screen->title);  // 화면 제목 출력

    for (int i = 0; i < screen->field_count; i++) {
        mvprintw(4 + i * 3, 10, "%s:", screen->fields[i].prompt);

		if (current_step == i) {
            // 현재 입력 중인 필드
            mvprintw(5 + i * 3, 12, "> %s", screen->fields[i].buffer);
        } else {
            // 다른 필드는 그대로 출력
            mvprintw(5 + i * 3, 12, "  %s", screen->fields[i].buffer);
        }
	}
	
	int height, width;
    getmaxyx(stdscr, height, width);
	(void) width;
    mvprintw(height - 1, 0, ":b return to previous page");
	
	refresh();
}
