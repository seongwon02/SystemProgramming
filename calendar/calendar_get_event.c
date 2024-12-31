#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include "calendar.h"
#include "calendar_get_event.h"

// 일정 정보를 저장하는 구조체
typedef struct info{
    char event_name[50];
	int start_week;
	int start_wday;
    int end_week;
    int end_wday;
	int start_year;
	int start_month;
	int start_day;
    char start_time[20];
    char end_time[20];
	int end_year;
	int end_month;
	int end_day;
    char memo[100];
    double quantity;
}info;

// 각 월의 일수를 저장한 배열 (윤년이 아닌 경우)
int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// 윤년인지 확인하는 함수
int is_leap_year(int year) {
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        return 1;
    }
    return 0;
}

// 연도와 월을 입력받아 해당 월의 첫 번째 요일을 계산하는 함수
// 0: 일요일, 1: 월요일, ..., 6: 토요일
// Zeller's Congruence를 사용하여 요일 계산
int get_start_day_of_month(int year, int month) {
    int day = 1; // 기준일
    int y = year, m = month;

    if (month < 3) {
        m += 12;
        y -= 1;
    }

	int k = y % 100; // 연도 뒷 2자리
	int j = y / 100; // 연도 앞 2자리
	
	// Zellar's Congruenc 공식 사용
    int h = (day + ((13 * (m + 1)) / 5) + k + (k / 4) + (j / 4) - (j * 2)) % 7;

	// 계산된 값을 0-6 범위로 조정(일~토)
	if (h == 0)
		return 6;
	else
		return h - 1;
}

// 일정 파일의 한 줄을 입력받아 구조체에 저장하는 함수
void get_line(char *line, struct info* e) {
    char *token = strtok(line, "|");

    // 일정 이름
    token = strtok(NULL, "|");
    if (token != NULL)
        strncpy(e->event_name, token, sizeof(e->event_name) - 1);


    // 시작 날짜
    token = strtok(NULL, "|");
    if (token != NULL) {
        sscanf(token, "%4d-%2d-%2d %5s", &e->start_year, &e->start_month, &e->start_day, e->start_time);
    } 
    // 종료 날짜
    token = strtok(NULL, "|");
    if (token != NULL) {
        sscanf(token, "%4d-%2d-%2d %5s", &e->end_year, &e->end_month, &e->end_day, e->end_time);
    }
    // 분량
    token = strtok(NULL, "|"); 
    token = strtok(NULL, "|"); 
    if (token != NULL) {
        sscanf(token, "%lf", &e->quantity);
    }
    // 필요 없는 데이터 건너뜀
    for (int i = 0; i < 4; i++) { 
        token = strtok(NULL, "|");
    }
    // 메모
    token = strtok(NULL, "|");
    if (token != NULL) {
        strncpy(e->memo, token, sizeof(e->memo) - 1);
        e->memo[sizeof(e->memo) - 1] = '\0'; // 널 종단 보장
    } else {
        strcpy(e->memo, "");
    }
}



// 일정 출력이 가능한 칸 찾기
int is_used(int week, int wday, int used_coordinate[6][7][3]) {

	if (used_coordinate[week][wday][0] == 1){
		return 0;
	}
    // 사용 가능한 줄 확인
    else if(used_coordinate[week][wday][1] ==  0){
        used_coordinate[week][wday][1] = 1;
        return 1;
    }
    else if(used_coordinate[week][wday][2] ==  0){
        used_coordinate[week][wday][2] = 1;
        return 2;
    }
    // 사용 가능한 줄이 없는 경우 0 리턴
    used_coordinate[week][wday][0] = 1;

	return 0;
}

// 출력 한도를 넘은 칸에 남은 일정 수 출력
void exceed_event(int row, int col,int day, int event_cnt[]) {
    event_cnt[day] += 1;
	
	attron(COLOR_PAIR(6));
	mvprintw(row, col + 3, "%d+", event_cnt[day]);
	attroff(COLOR_PAIR(6));
}

// 연속 일정을 막대로 표현하는 함수
int manage_schedule_cell(int op, int done, int row, int col, int color, char* name, char*blank) {
               
    if (!done) // 일정 이름이 출력되지 않았다면
    {
        attron(COLOR_PAIR(color));
        mvprintw(row + op, col, "%s", name);
        done = 1;
        attroff(COLOR_PAIR(color));
    }
    else // 일정 이름이 출력된 경우
    {
        attron(COLOR_PAIR(color));
        mvprintw(row + op, col, "%s", blank);
        attroff(COLOR_PAIR(color));
        done++;
    }
    return done;       
}

// 표 형식의 일정 출력
int print_event_table(struct info* event, int width, int x, int y_coordinate[3])
{
    char name[width+1];

    // 칸에 표현가능한 문자열인 경우 정상출력, 아니면 문자열 자르고 뒤에 ..표시
    if (strlen(event->event_name) <= width+1){
		snprintf(name, width+1, "%-*s", width, event->event_name);
    }    
	else {
		char temp[width+1];
		strncpy(temp, event->event_name, width - 2);
		temp[width - 2] = '\0';  
		snprintf(name, width, "%s..", temp);
	}

    // 일 기준 캘린더인 경우
    if (state == DAILY_CALENDAR)
    {
        mvprintw(x, y_coordinate[0], "%s", name);
        if (event->quantity != -1)
            mvprintw(x, y_coordinate[0]-7, "[%.2lf]", event->quantity);

        // 시작시간, 종료시간이 표기되어 있는 경우
        if (strcmp(event->start_time, "24:00") != 0 && strcmp(event->end_time, "24:00") != 0)  
            mvprintw(x, y_coordinate[1] + width / 2 - 6, "%s~%s", event->start_time, event->end_time);
        // 시작시간만 표기된 경우
        else if(strcmp(event->start_time, "24:00") != 0 && strcmp(event->end_time, "24:00") == 0) 
            mvprintw(x, y_coordinate[1] + width / 2 - 2, "%s~", event->start_time);
        // 종료 시간만 표기된 경우
        else if(strcmp(event->end_time, "24:00") != 0 && strcmp(event->start_time, "24:00") == 0) 
            mvprintw(x, y_coordinate[1] + width / 2 - 2, "~%s", event->end_time);
        // 시간이 표기되어 있지 않은 경우
        else if(strcmp(event->start_time, "24:00") == 0) 
            mvprintw(x, y_coordinate[1] + width / 2 - 1, "-");

        // 메모을 칸 크기에 맞추어 출력
        if(strlen(event->memo) > 0)
        {
            for (int i = 0; i < strlen(event->memo); i += width * 2)
            {
                // 화면의 높이를 초과하면 중단
                if (x >= LINES - 3) {
                    break;
                }

                mvwprintw(stdscr, x, y_coordinate[2] + 1, "%.*s", width * 2, &event->memo[i]);
                x++;
            }
        }
        else
            x++;
    }
    else if (state == MONTHLY_CALENDAR) // 표 형식의 월 기준 캘린더인 경우
    {
        // 단일일정인 경우
        if (event->start_year == event->end_year &&
            event->start_month == event->end_month &&
            event->start_day == event->end_day)
            mvprintw(x, y_coordinate[0], "%4d-%02d-%02d", event->start_year, event->start_month, event->start_day);
        // 연속 일정의 경우
        else
            mvprintw(x, y_coordinate[0], "%4d-%02d-%02d~%4d-%02d-%02d", 
            event->start_year, event->start_month, event->start_day, event->end_year, event->end_month, event->end_day);

        mvprintw(x, y_coordinate[1], "%s", name);
        
        // 시작시간, 종료시간이 표기되어 있는 경우
        if (strcmp(event->start_time, "24:00") != 0 && strcmp(event->end_time, "24:00") != 0)  
            mvprintw(x, y_coordinate[2] + width / 3 - 6, "%s~%s", event->start_time, event->end_time);
        // 시작시간만 표기된 경우
        else if(strcmp(event->start_time, "24:00") != 0 && strcmp(event->end_time, "24:00") == 0) 
            mvprintw(x, y_coordinate[2] + width / 3 - 2, "%s~", event->start_time);
        // 종료 시간만 표기된 경우
        else if(strcmp(event->end_time, "24:00") != 0 && strcmp(event->start_time, "24:00") == 0) 
            mvprintw(x, y_coordinate[2] + width / 3- 2, "~%s", event->end_time);
        // 시간이 표기되어 있지 않은 경우
        else if(strcmp(event->start_time, "24:00") == 0) 
            mvprintw(x, y_coordinate[2] + width / 3- 1, "-");

    }

    return x;
}

// 주 기준 캘랜더에 일정 출력
int print_event_week(struct info *e, const int width, int x, int cnt, int y_coordinate[], int used_coordinate[][7]) 
{
    char name[width+1];
    char blank[width+1];
    memset(blank, ' ', width+1);
    blank[width] = '\0';

    // 칸 크기에 맞는 문자열이면 정상출력, 아니면 문자열 자르고 .. 추가
    if (strlen(e->event_name) <= width+1) {
        snprintf(name, width+1, "%-*s", width, e->event_name);
    }
	else {
        char temp[width + 1];
		strncpy(temp, e->event_name, width - 2);
		temp[width - 2] = '\0';  
		snprintf(name, width+1, "%s..", temp);
	}
    
    if (e->end_day != 0){   // 연속 일정인 경우
        for (int i = e->end_wday; i >= e->start_wday ; i--)
        {
            int row = x;
            int col = y_coordinate[i];

            attron(COLOR_PAIR(color+7));
            mvprintw(row, col, "%s", blank); // 막대
            
            if(i == e->start_wday) // 일정 이름을 표시하는 경우
            {    
                if (e->start_wday == 6)
                    mvprintw(row, col, "%s", name);
                else
                {
                    mvprintw(row, col, "%s", e->event_name);
                }
            }
            attroff(COLOR_PAIR(color+7));
        }
        x++;
        return x; 
    }
    else // 단일 일정인 경우
    {
        for (int i = cnt; i < 9; i++)
        {
            if (!used_coordinate[i][e->start_wday])
            {
                int row =  x + i - cnt;
                int col = y_coordinate[e->start_wday];
                mvprintw(row, col, "%s", name);

                used_coordinate[i][e->start_wday] = 1;
                return x;
            }          
        }
        
    }
    return x;
}


// 달력에 연속일정을 출력하는 함수
void print_continuous_event(struct info *event, const int width, int color, int x_coordinate[6][7], int y_coordinate[6][7], int used_coordinate[6][7][3], int event_cnt[]) {
	char name[width+1];
    char temp[width+1];
	int op = -1, done = 0;
    int day = event->start_day;
    int row, col;
    int s_row, s_col, s_wday, s_op; // 일정 출력할 좌표와 요일

    // 칸 크기에 맞는 문자열이면 정상출력, 아니면 문자열 자르고 .. 추가
	if (strlen(event->event_name) <= width+1) {
        snprintf(name, width+1, "%-*s", width, event->event_name);
    }
	else {
		strncpy(temp, event->event_name, width - 2);
		temp[width - 2] = '\0';  
		snprintf(name, width+1, "%s..", temp);
	}

    memset(temp, ' ', width+1);
    temp[width] = '\0';
    
    // 일정 출력
    for (int i = event->start_week; i <= event->end_week; i++)
    {
        if (i == event->start_week) // 첫째 주인 경우
        {   
            int end = 6;
            if (event->start_week == event->end_week)
                end = event->end_wday;
            for (int j = event->start_wday; j <= end; j++)
            {
                row = x_coordinate[i][j];
                col = y_coordinate[i][j];

                // 사용 가능한 칸 확인 
                if (op < 1)
                    op = is_used(i, j, used_coordinate);

                if (op == 0) { // 초과 일정으로 표시
                    exceed_event(row, col, day, event_cnt);
                }
                else {
                    done = manage_schedule_cell(op, done, row, col, color, name, temp);
                }
                if (done == 1)
                {
                    s_row = row; s_col = col; s_wday = j; s_op = op;
                }  
                used_coordinate[i][j][op] = 1; // 줄 사용 표시
                day++;  
                
            }
            if (done){
                if (s_wday == 6)
                    done = manage_schedule_cell(s_op, 0, s_row, s_col, color, name, temp);
                else
                    done = manage_schedule_cell(s_op, 0, s_row, s_col, color, event->event_name, temp);
            }
                
        }
        else if (i == event->end_week) // 마지막 주인 겨우
        {
            for (int j = 0; j <= event->end_wday; j++)
            {
                row = x_coordinate[i][j];
                col = y_coordinate[i][j];

                // 사용 가능한 칸 확인 
                if (op < 1)
                    op = is_used(i, j, used_coordinate);
                
                if (op == 0) // 초과 일정
                    exceed_event(row, col, day, event_cnt);
                else // 막대 표시
                {
                    done = manage_schedule_cell(op, done, row, col, color, event->event_name, temp);
                }
                if (done == 1)
                {
                    s_row = row; s_col = col; s_wday = j;
                }  
                used_coordinate[i][j][op] = 1; // 줄 사용 표시
                day++;  
                
            }
            if (done){
                if (s_wday == 6)
                    done = manage_schedule_cell(s_op, 0, s_row, s_col, color, name, temp);
                else
                    done = manage_schedule_cell(s_op, 0, s_row, s_col, color, event->event_name, temp);
            }
        }
        else // 나머지 주들
        {
            for (int j = 0; j < 7; j++)
            {
                row = x_coordinate[i][j];
                col = y_coordinate[i][j];

                // 사용 가능한 칸인지 확인
                if (op < 1)
                    op = is_used(i, j, used_coordinate);
                
                if (op == 0) // 초과 일정으로 표시
                    exceed_event(row, col, day, event_cnt);
                else // 막대 표시
                {
                    done = manage_schedule_cell(op, done, row, col, color, event->event_name, temp);
                }

                if (done == 1)
                {
                    s_row = row; s_col = col; s_wday = j; s_op = op;
                }  
                used_coordinate[i][j][op] = 1; // 줄 사용 표시
                day++;  
            }
            if (done){
                if (s_wday == 6)
                    done = manage_schedule_cell(s_op, 0, s_row, s_col, color, name, temp);
                else
                    done = manage_schedule_cell(s_op, 0, s_row, s_col, color, event->event_name, temp);
            }
        }
    }


	return;
}

// 달력에 단일일정을 출력하는 함수
void print_single_event(struct info *event, const int width, int x_coordinate[6][7], int y_coordinate[6][7], int used_coordinate[6][7][3], int event_cnt[]) {
	char name[width+1];
	int row, col, op;

    // 칸 크기에 맞는 문자열이면 정상출력, 아니면 문자열 자르고 .. 추가
	if (strlen(event->event_name) <= width+1){
		snprintf(name, width+1, "%-*s", width, event->event_name);
    }    
	else {
		char temp[width+1];
		strncpy(temp, event->event_name, width - 2);
		temp[width - 2] = '\0';  
		snprintf(name, width, "%s..", temp);
	}
	
	row = x_coordinate[event->start_week][event->start_wday];
    col = y_coordinate[event->start_week][event->start_wday];

    // 사용 가능한 칸인지 확인
    op = is_used(event->start_week, event->start_wday, used_coordinate);

    if (op == 0) { // 초과 일정으로 표시
	    event_cnt[event->start_day] += 1;
        attron(COLOR_PAIR(6));
	    mvprintw(row, col + 3, "%d+", event_cnt[event->start_day]);
	    attroff(COLOR_PAIR(6));
    }
    else // 일정 표시
    {
        mvprintw(row + op, col, "%s", name);
    }


	return;
}

// 연속 일정을 읽고 달력에 출력하는 함수
void get_continuous_event(int year, int month, int start_wday, int width, int x_coordinate[6][7], int y_coordinate[6][7], int used_coordinate[6][7][3], int event_cnt[32]) {
	info e;
    char line[256];
    char ch;
    int n_char, index = 0;

    // 파일 열기
    int fd = open(EVENT_FILE, O_RDONLY);
    if (fd == -1) {
        endwin();
        fprintf(stderr, "Error: 일정을 불러올 수 없습니다.\n");
        perror(EVENT_FILE);
        exit(1);
    }

    // 파일 읽기
    while ((n_char = read(fd, &ch, 1)) >= 0) {
        if (n_char == 0 || ch == '\n' || index >= 255) {
            line[index] = '\0'; 
            index = 0;

            if (strlen(line) < 10) // 파일의 첫번째 줄 무시
            {
                if (n_char == 0)
                    break;
                else
                    continue;
            }
             // 한 줄 파싱
            get_line(line, &e);

            // 연속일정인지 확인
            if (e.start_year == e.end_year && e.start_month == e.end_month && e.start_day == e.end_day){
                if (n_char == 0)
                    break;
                else
                    continue;
            }

            int start_eNum = e.start_year * 100 + e.start_month;
            int end_eNum = e.end_year * 100 + e.end_month;
            int cal_Num = year * 100 + month;

            if (cal_Num == start_eNum && cal_Num == end_eNum) // 연속일정이 현재 달에만 하는 경우
            {
                e.start_wday = (e.start_day + start_wday - 1) % 7; // 일정 시작하는 요일 계산
                e.start_week = (e.start_day + start_wday - 1) / 7; // 일정 시작하는 주 계산
                e.end_week = (e.end_day + start_wday - 1) / 7; // 일정 끝나는 주 계산
                e.end_wday = (e.end_day + start_wday - 1) % 7; // 일정 끝나는 요일 계산

                print_continuous_event(&e, width, color + 7, x_coordinate, y_coordinate, used_coordinate, event_cnt);
                color = ((color + 1) % 3);     
            }
            else if (cal_Num == start_eNum && cal_Num < end_eNum) // 일정이 다음달까지 이어지는 경우
            {
                int days = days_in_month[month-1];
                // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }
    
                // 일정 끝나는 요일 계산
                if (month != 12)
                    e.end_wday = get_start_day_of_month(year, month + 1) - 1;
                else
                    e.end_wday = get_start_day_of_month(year+1, 1) - 1;

                if (e.end_wday < 0)
                    e.end_wday = 6;

                e.end_week = (days + start_wday - 1) / 7; //일정 끝나는 주 계산

                e.start_week = (e.start_day + start_wday - 1) / 7; // 일정 시작하는 주 계산
                e.start_wday = (e.start_day + start_wday - 1) % 7; // 일정 시작하는 요일 계산

                print_continuous_event(&e, width, color + 7, x_coordinate, y_coordinate, used_coordinate, event_cnt);
                color = ((color + 1) % 3);     
            }
            else if(cal_Num > start_eNum && cal_Num < end_eNum) // 이번달 전체가 일정인 경우
            {
                int days = days_in_month[month-1];
                // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }
    
                // 일정 끝나는 요일 계산
                if (month != 12)
                    e.end_wday = get_start_day_of_month(year, month + 1) - 1;
                else
                    e.end_wday = get_start_day_of_month(year+1, 1) - 1;

                if (e.end_wday < 0)
                    e.end_wday = 6;

                e.end_week = (days + start_wday - 1) / 7; //일정 끝나는 주 계산

                e.start_wday = get_start_day_of_month(year, month); // 일정 시작하는 요일 계산
                e.start_week = 0; // 일정 시작하는 주 계산

                print_continuous_event(&e, width, color + 7, x_coordinate, y_coordinate, used_coordinate, event_cnt);
                color = ((color + 1) % 3);     
            }
            else if (cal_Num > start_eNum && cal_Num == end_eNum) // 일정의 마지막 달인 경우
            {
                e.start_wday = get_start_day_of_month(e.end_year, e.end_month); // 일정 시작하는 요일 계산
                e.start_week = 0; // 일정 시작하는 주 계산
                e.end_week = (e.end_day + e.start_wday - 1) / 7; // 일정 끝나는 주 계산
                e.end_wday = (e.end_day + e.start_wday - 1) % 7; // 일정 끝나는 요일 계산

                print_continuous_event(&e, width, color + 7, x_coordinate, y_coordinate, used_coordinate, event_cnt);
                color = ((color + 1) % 3);     
            }    
       
            if (n_char == 0)
            {
                break;
            }
        }
        else {
            line[index++] = ch; // 버퍼에 문자 추가
        }         
    }

    if (n_char == -1) {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        exit(1);
    }

    if (close(fd) == -1) // 파일 닫기
    {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        endwin();
    }
}

// 단일 일정을 읽고 달력에 출력하는 함수
void get_single_event(int year, int month, int start_wday, int width, int x_coordinate[6][7], int y_coordinate[6][7], int used_coordinate[6][7][3], int event_cnt[32]) {
	info e;
    char line[256];
    char ch;
    int n_char, index = 0;

    // 파일 열기
    int fd = open(EVENT_FILE, O_RDONLY);
    if (fd == -1) {
        endwin();
        fprintf(stderr, "Error: 일정을 불러올 수 없습니다.\n");
        perror(EVENT_FILE);
        exit(1);
    }

    // 파일 읽기
    while ((n_char = read(fd, &ch, 1)) > 0) {
        if (n_char == 0 || ch == '\n' || index >= 255) {
            line[index] = '\0'; 
            index = 0;
            
            if (strlen(line) < 10) // 파일의 첫번째 줄 무시
            {
                if (n_char == 0)
                    break;
                else
                    continue;
            }

            get_line(line, &e);

            // 단일일정인지 확인
            if (e.start_year != e.end_year || e.start_month != e.end_month || e.start_day != e.end_day) 
            {
                if (n_char == '\0')
                    break;
                else
                    continue;
            }

            // 이번 달 일정인지 확인
            if (e.start_year == year && e.start_month == month) {
                e.start_week = (e.start_day + start_wday - 1) / 7; // 주차 계산
                e.start_wday = (e.start_day + start_wday - 1) % 7;// 요일 계산

                print_single_event(&e, width, x_coordinate, y_coordinate, used_coordinate, event_cnt);
            }

            if (n_char == 0)
            {
                break;
            }
        }
        else
        {
            line[index++] = ch;
        }
        
    }

    if (n_char == -1) {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        exit(1);
    }

    if (close(fd) == -1) // 파일 닫기
    {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        endwin();
    }
}

void get_event_week(int year, int month, int start_day, int end_day, int width, int x, int y_coordinate[7], int used_coordinate[][7]) {
	info e;
    char line[256];
    int cnt = -1;
    char ch;
    int n_char, index = 0;

    // 파일 열기
    int fd = open(EVENT_FILE, O_RDONLY);
    if (fd == -1) {
        endwin();
        fprintf(stderr, "Error: 일정을 불러올 수 없습니다.\n");
        perror(EVENT_FILE);
        exit(1);
    }

    // 연속파일 읽기
    while ((n_char = read(fd, &ch, 1)) >= 0) {
        if (n_char == 0 || ch == '\n' || index >= 255) {
            line[index] = '\0'; 
            index = 0;

            if (strlen(line) < 10) // 파일의 첫번째 줄 무시
            {
                if (n_char == 0)
                    break;
                else
                    continue;
            }

            // 한 줄 파싱
            get_line(line, &e);

            // 연속일정인지 확인
            if (e.start_year == e.end_year && e.start_month == e.end_month && e.start_day == e.end_day)
            {
                if (n_char == '\0')
                    break;
                else
                    continue;
            }

            // 일정이 현재 주에 포함되는지 확인
            int start_date = e.start_year * 10000 + e.start_month * 100 + e.start_day;
            int end_date = e.end_year * 10000 + e.end_month * 100 + e.end_day;
            int w_start_date = year * 10000 + month * 100 + start_day;
            int w_end_date = (start_day < end_day) ? year * 10000 + month * 100 + end_day : 
                            (e.end_month == 1) ? (year + 1) * 10000 + (1) * 100 + end_day : (year) * 10000 + (month+1) * 100 + end_day;
            
            if (start_date < w_start_date && w_end_date < end_date) // 일정이 현재 주에 포함되고 일정의 시작일과 종료일이 주 안에 포함되지 핞는 경우
            {
                e.start_wday = 0;
                e.end_wday = 6;
                x = print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                cnt++;
                color = ((color + 1) % 3);
            }
            else if ((start_date >= w_start_date && start_date <= w_end_date) && w_end_date < end_date) // 일정의 시작일만 현재 주에 포함되는 경우
            {
                int days = days_in_month[month-1];
                    // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }

                // 달이 넘어가는 주와 아닌 주를 구분
                if (e.start_month == month)
                    e.start_wday = e.start_day - start_day;
                else if (e.start_month == month+1 || e.start_month == 1)
                    e.start_wday = days - start_day + e.start_day;
                
                e.end_wday = 6;

                x = print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                cnt++;
                color = ((color + 1) % 3);
            }
            else if (start_date < w_start_date && (end_date >= w_start_date && end_date <= w_end_date)) // 일정의 종료일만 현재 주에 포함되는 경우
            {
                int days = days_in_month[month-1];
                    // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }

                // 달이 넘어가는 주와 아닌 주를 구분
                if (e.end_month == month)
                    e.end_wday = e.end_day - start_day;
                else if (e.end_month == month+1 || e.end_month == 1)
                    e.end_wday = days - start_day + e.end_day;
                
                e.start_wday = 0;
                
                x = print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                cnt++;
                color = ((color + 1) % 3);
            }
            else if (start_date >= w_start_date && w_end_date >= end_date) // 일정의 시작/종료일이 현재 주에 포함되는 경우
            {
                int days = days_in_month[month-1];
                    // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }

                // 달이 넘어가는 주와 아닌 주를 구분
                if (e.start_month == month)
                    e.start_wday = e.start_day - start_day;
                else if (e.start_month == month+1 || e.start_month == 1)
                    e.start_wday = days - start_day + e.start_day;

                if (e.end_month == month)
                    e.end_wday = e.end_day - start_day;
                else if (e.end_month == month+1 || e.end_month == 1)
                    e.end_wday = days - start_day + e.end_day;
                
                x = print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                cnt++;
                color = ((color + 1) % 3);
            }
            if (n_char == 0)
            {
                break;
            }
        }
        else{
            line[index++] = ch;
        }
    }

    if (n_char == -1) {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        exit(1);
    }

    // 단일 일정
    if (lseek(fd, 0, SEEK_SET) == -1) {
        endwin();
        fprintf(stderr, "Error: 일정을 불러올 수 없습니다.\n");
        close(fd);
        perror(EVENT_FILE);
        exit(1);
    }

    //한줄씩 파일 읽기
    while ((n_char = read(fd, &ch, 1)) > 0) {
        if (n_char == 0 || ch == '\n' || index >= 255) {
            line[index] = '\0'; 
            index = 0;

            if (strlen(line) < 10) // 파일의 첫번째 줄 무시
            {
                if (n_char == 0)
                    break;
                else
                    continue;
            }

            // 한 줄 파싱
            get_line(line, &e);

            // 단일일정인지 확인
            if (e.start_year != e.end_year || e.start_month != e.end_month || e.start_day != e.end_day)
            {
                if (n_char == '\0')
                    break;
                else
                    continue;
            }

            if (start_day < end_day) // 달이 넘어가는 주가 아닌 경우
            {
                if ((e.start_year == year && e.start_month == month) &&
                    (e.start_day >= start_day && e.start_day <= end_day))
                {
                    e.start_wday = e.start_day - start_day;
                    e.end_day = 0;
                    print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                    mvprintw(0,0,"0");
                }     
            }
            else // 넘어가는 주인 경우
            {
                int days = days_in_month[month-1];
                    // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }
                
                if ((e.start_year == year && e.start_month == month) &&
                    (e.start_day >= start_day && e.start_day <= days)) // 일정이 달을 넘어가지 않는 경우
                {
                    e.start_wday = e.start_day - start_day;
                    e.end_day = 0;
                    print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                    mvprintw(1,0,"1");

                }
                else if (((e.start_year == year && e.start_month == month + 1) || (e.start_year == year + 1 && e.start_month == 1)) &&
                        (e.start_day >= 1 && e.start_day <= end_day)) // 일정이 달을 넘는 경우
                {
                    e.start_wday = days - start_day + e.start_day;
                    e.end_day = 0;
                    print_event_week(&e, width, x, cnt, y_coordinate, used_coordinate);
                }
            }
            if (n_char == 0)
            {
                break;
            }
        }
        else
        {
            line[index++] = ch;
        }
    }

    if (n_char == -1) {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        exit(1);
    }

    if (close(fd) == -1) // 파일 닫기
    {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        endwin();
    }
}

// 표 형식의 캘린더에 일정을 가져와 출력하는 함수
void get_event_in_table(int year, int month, int day, int width, int x, int y_coordinate[])
{
	info e;
    char line[256];
    char ch;
    int n_char, index = 0;

    // 파일 열기
    int fd = open(EVENT_FILE, O_RDONLY);
    if (fd == -1) {
        endwin();
        fprintf(stderr, "Error: 일정을 불러올 수 없습니다.\n");
        perror(EVENT_FILE);
        exit(1);
    }

    // 연속파일 읽기
    while ((n_char = read(fd, &ch, 1)) >= 0) {
        if (n_char == 0 || ch == '\n' || index >= 255) {
            line[index] = '\0'; 
            index = 0;

            if (strlen(line) < 10) // 파일의 첫번째 줄 무시
            {
                if (n_char == 0)
                    break;
                else
                    continue;
            }
            // 한 줄 파싱
            get_line(line, &e);

            // 연속일정인지 확인
            if (e.start_year == e.end_year && e.start_month == e.end_month && e.start_day == e.end_day)
            {
                if (n_char == '\0')
                    break;
                else
                    continue;
            }

            if (state == DAILY_CALENDAR){ // 일간 캘린더
                // 일정이 오늘 포함되는지 확인
                int start_date = e.start_year * 10000 + e.start_month * 100 + e.start_day;
                int end_date = e.end_year * 10000 + e.end_month * 100 + e.end_day;
                int current_date = year * 10000 + month * 100 + day;

                // 분량 계산
                if (e.quantity != -1)
                {
                    e.quantity = e.quantity * (day - e.start_day + 1);
                }

                if (current_date >= start_date && current_date <= end_date) {
                    x = print_event_table(&e, width, x, y_coordinate);
                }
            }
            else if (state == MONTHLY_CALENDAR) // 월간 캘린더
            {   
                
                // 일정이 이번달에 포함되는지 확인
                int days = days_in_month[month-1];
                    // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }

                int start_date = e.start_year * 10000 + e.start_month * 100 + e.start_day;
                int end_date = e.end_year * 10000 + e.end_month * 100 + e.end_day;
                int m_start_date = year * 10000 + month * 100 + 1;
                int m_end_date = year * 10000 + month * 100 + days;

                if (!(start_date > m_end_date || end_date < m_start_date))
                {
                    x = print_event_table(&e, width, x, y_coordinate);
                    x++;
                }
            }

            if (n_char == 0)
            {
                break;
            }
        }
        else
        {
            line[index++] = ch;
        }
    }
   
    if (n_char == -1) {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        exit(1);
    }

    // 단일 일정
    if (lseek(fd, 0, SEEK_SET) == -1) {
        endwin();
        fprintf(stderr, "Error: 일정을 불러올 수 없습니다.\n");
        close(fd);
        perror(EVENT_FILE);
        exit(1);
    }

    //한줄씩 파일 읽기
    while ((n_char = read(fd, &ch, 1)) > 0) {
        if (n_char == 0 || ch == '\n' || index >= 255) {
            line[index] = '\0'; 
            index = 0;

            if (strlen(line) < 10) // 파일의 첫번째 줄 무시
            {
                if (n_char == 0)
                    break;
                else
                    continue;
            }
            // 한 줄 파싱
            get_line(line, &e);
            
            // 단일일정인지 확인
            if (e.start_year != e.end_year || e.start_month != e.end_month || e.start_day != e.end_day)
            {
                if (n_char == '\0')
                    break;
                else
                    continue;
            }

            // 이번 달 일정인지 확인
            if(state == DAILY_CALENDAR){
                if (e.start_year == year && e.start_month == month && e.start_day == day) {
                    
                    x = print_event_table(&e, width, x, y_coordinate);
                }
            }
            else if (state == MONTHLY_CALENDAR)
            {
                // 일정이 이번달에 포함되는지 확인
                int days = days_in_month[month-1];
                
                // 윤년이고 2월인 경우, 하루를 추가
                if (month == 2 && is_leap_year(year)) {
                    days = 29;
                }

                int start_date = e.start_year * 10000 + e.start_month * 100 + e.start_day;
                int m_start_date = year * 10000 + month * 100 + 1;
                int m_end_date = year * 10000 + month * 100 + days;

                if (m_start_date <= start_date && start_date <= m_end_date)
                {
                    x = print_event_table(&e, width, x, y_coordinate);
                    x++;
                }
            } 
            if (n_char == 0)
            {
                break;
            }
        }
        else{
            line[index++] = ch;
        }
    }

    if (n_char == -1) {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        exit(1);
    }

    if (close(fd) == -1) // 파일 닫기
    {
        endwin();
        fprintf(stderr, "Error: %s", EVENT_FILE);
        endwin();
    }
}
