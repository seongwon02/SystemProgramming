#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "date_check.h"

static pthread_t date_monitor_thread;
static int running = 1;
static pid_t main_process_pid;


// 현재 날짜 가져오기
static void getCurrentDate(char *date) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(date, 11, "%Y-%m-%d", tm_info);
}

static char previous_date[11] = {0}; // 바뀌기 전 날짜를 저장하는 전역 변수

static void *dateMonitorThread(void *arg) {
    char last_checked_date[11] = {0};
    getCurrentDate(last_checked_date);
    strcpy(previous_date, last_checked_date); // 초기화 시 현재 날짜 저장

    while (running) {
        sleep(10);

        char current_date[11];
        getCurrentDate(current_date);
        if (strcmp(last_checked_date, current_date) != 0) {

            strcpy(previous_date, last_checked_date); // 날짜 변경 전 상태 저장
            strcpy(last_checked_date, current_date);  // 새로운 날짜로 업데이트

            if (main_process_pid > 0) {
                kill(main_process_pid, SIGUSR1); // 날짜 변경시 메인 프로세서에 시그널보냄
            }
        }
    }

    return NULL;
}



// 날짜 감지 스레드 초기화
void initializeDateMonitor() {
    main_process_pid = getpid();
}

// 날짜 감지 스레드 시작
void startDateMonitor() {
    running = 1;
    pthread_create(&date_monitor_thread, NULL, dateMonitorThread, NULL);
}

// 날짜 감지 스레드 종료
void stopDateMonitor() {
    running = 0;
    pthread_join(date_monitor_thread, NULL);
}
