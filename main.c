#include <ncurses.h>
#include <signal.h>
#include "global.h"
#include "util.h"
#include "display.h"
#include "event.h"
#include "habit.h"
#include "scheduler.h"
#include "date_check.h"
#include "./calendar/calendar.h"


void handleDateChangeSignal(int signal, siginfo_t *info, void *context) {
    if (signal == SIGUSR1) {
        
        signalresetHabits();
        updateDdayAndWeights(events, event_count);
        draw_title();
        draw_main_menu();
        draw_lists();       
    }
}


int main() {
    loadEvents();
    loadHabits();
    
    updateDdayAndWeights(events, event_count);
    struct sigaction new_handler;

    new_handler.sa_sigaction = handleDateChangeSignal;//시그널발생시 처리 함수
    new_handler.sa_flags = 0; //일단 0으로 세팅
    sigemptyset(&new_handler.sa_mask); 
    
    if (sigaction(SIGUSR1, &new_handler, NULL) == -1) {
        perror("sigaction error");
        return 1;
    }
    initializeDateMonitor();
    startDateMonitor();

    initscr();         // ncurses 초기화
    noecho();          // 사용자 입력 숨기기
    curs_set(FALSE);   // 커서 숨기기

    signal(SIGWINCH, handle_resize); // 터미널 크기 변경 감지

    draw_title();           // 타이틀 출력
    draw_main_menu();       // 초기 메뉴 출력
    draw_lists();           // 초기 리마인더 및 습관 목록 출력

    while (1) {
        int choice = getch(); // 사용자 입력 대기

        switch (choice) {
            case '1': // 캘린더 출력
				current_screen = CALENDAR_SCREEN; // 캘린더 화면과 메인화면 창 크기 변경 시 실행 구분 위함
                draw_calendar();        // 캘린더 출력
                current_screen = MAIN_SCREEN; // 초기 화면으로 복귀
                clear();
                nodelay(stdscr, FALSE); // getch()를 차단 모드로 설정
		
                draw_title();           // 타이틀 출력
                draw_main_menu();       // 메뉴 유지
                draw_lists();           // 리스트 갱신
				break;
            case '2': // 이벤트 관리
                event_submenu();
                current_screen = MAIN_SCREEN; // 초기 화면으로 복귀
                clear();
                				
                draw_title();          
                draw_main_menu();
                draw_lists();
				break;
            case '3': // 습관 관리
                habit_submenu();
				current_screen = MAIN_SCREEN; // 초기 화면으로 복귀
                clear();
                				
                draw_title();          
                draw_main_menu();
                draw_lists();
                break;
            case 'q': // 프로그램 종료
                // 현재 file IO로 인해 종료까지 멈춤 현상
                // 종료 로딩창 or 입력 방지 등 필요해 보임
                saveHabits();
                stopDateMonitor();
                endwin();
                return 0;
            default:
                break;
        }
    }

    endwin(); // ncurses 종료
    return 0;
}
