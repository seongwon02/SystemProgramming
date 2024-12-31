#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "event.h"
#include "global.h"
#include "util.h"
#include "scheduler.h"
#include "display.h"

// 이벤트 관리 서브 메뉴
void event_submenu() {
	current_screen = EVENT_SCREEN;

	int choice;
	
	while (1) {
        draw_event_screen();

        choice = getch();  // 사용자 입력 대기

        if (choice == '1') {
            addEvents();  // 일정 추가
			clear();

			draw_event_screen();
        } else if (choice == '2') {
            modifyEvents();  // 일정 수정
			clear();

			current_screen = EVENT_SCREEN;
			draw_event_screen();
        } else if (choice == '3') {
            deleteEvents();  // 일정 삭제
			clear();

			current_screen = EVENT_SCREEN;
			draw_event_screen();
		} else if (choice == '4') {
			add_schedule();  // 오토 스케줄링
			clear();

			current_screen = EVENT_SCREEN;
			draw_event_screen();
        } else if (choice == '5') {
            break;  // 초기화면 복귀
        }
        // 변경된 event 및 last_event_id 를 txt에 바로 반영
        saveEvents();
    }
}

// 일정 txt에서 불러오기
void loadEvents() {
    FILE *file = fopen(EVENT_FILE, "r");
    if (file == NULL) {
        // 파일이 존재하지 않는 경우, 파일 생성 및 초기 세팅
        file = fopen(EVENT_FILE, "w");
        if (!file) {
            perror("파일 생성 실패");
            return;
        }
        
        last_event_id = 0;
        event_count = 0;
        return;
    }

    char buffer[512];
    event_count = 0;
	
    // 파일이 존재하는 경우, 파일에서 값 읽기
    if (fscanf(file, "%d", &last_event_id) != 1) {   // 읽을 값이 없는 경우
        last_event_id = 0;
    }
    else // 읽을 값이 있는 경우
    {
        fgets(buffer, sizeof(buffer), file);
    }

    while (fgets(buffer, sizeof(buffer), file) && event_count < MAX_EVENTS) {
        Event *event = &events[event_count];
        sscanf(buffer,
               "%d|%49[^|]|%d-%d-%d %d:%d|%d-%d-%d %d:%d|%d|%lf|%lf|%lf|%lf|%d|%99[^\n]",
               &event->id, event->title,
               &event->date_start.year, &event->date_start.month, &event->date_start.day,
               &event->date_start.hour, &event->date_start.minute,
               &event->date_end.year, &event->date_end.month, &event->date_end.day,
               &event->date_end.hour, &event->date_end.minute,
               &event->importance, &event->quantity, &event->interval,
               &event->Dday, &event->weight, &event->reminder,
               event->details);

        event_count++;
    }

    fclose(file);
}

// 일정 txt에 저장하기
void saveEvents() {
    FILE *file = fopen(EVENT_FILE, "w");
    if (file == NULL) {
        perror("파일 생성 실패");
        return;
    }

    // 첫 줄: 마지막 Event ID
    fprintf(file, "%d\n", last_event_id);

    for (int i = 0; i < event_count; i++) {
        Event *event = &events[i];
        fprintf(file, "%d|%s|%d-%02d-%02d %02d:%02d|%d-%02d-%02d %02d:%02d|%d|%.2lf|%.2lf|%.2lf|%.2lf|%d|%s\n",
                event->id, event->title,
                event->date_start.year, event->date_start.month, event->date_start.day,
                event->date_start.hour, event->date_start.minute,
                event->date_end.year, event->date_end.month, event->date_end.day,
                event->date_end.hour, event->date_end.minute,
                event->importance, event->quantity, event->interval,
                event->Dday, event->weight, event->reminder,
                event->details);
    }

    fclose(file);
}

// 일정 추가
void addEvents() {
    if (event_count >= MAX_EVENTS) {
        popup_message("Event list is full. Cannot add more events.");
        return;
    }

    Event event = {0}; // 초기화
    Time current;
    initTime(&current); // 현재 시간 초기화

    char start_date[20] = {0}, start_time[10] = {0};
    char end_date[20] = {0}, end_time[10] = {0};
    char reminder[5] = {0};
    char details[100] = {0};

    // 입력 필드 정의
    InputField fields[] = {
        {"Enter event title", event.title, sizeof(event.title), validate_title, 0},
        {"Enter start date (YYYY MM DD)", start_date, sizeof(start_date), validate_date_wrapper, 0},
        {"Enter start time (HH MM)", start_time, sizeof(start_time), validate_time_wrapper, 1}, // 공백 허용
        {"Enter end date (YYYY MM DD)", end_date, sizeof(end_date), validate_date_wrapper, 0},
        {"Enter end time (HH MM)", end_time, sizeof(end_time), validate_time_wrapper, 1}, // 공백 허용
        {"Set reminder (1: Yes, 0: No)", reminder, sizeof(reminder), validate_reminder, 0},
        {"Enter event details", details, sizeof(details), NULL, 1} // 공백 허용, 세부 사항은 유효성 검사 없음
    };

    // UI 화면 정의
    UIScreen screen = {"Add Event", fields, sizeof(fields) / sizeof(fields[0])};

	// 공통 입력 처리 함수 호출
    if (process_user_input(&screen) == 0) {
        // 데이터 파싱 및 유효성 검사
        sscanf(start_date, "%d %d %d", &event.date_start.year, &event.date_start.month, &event.date_start.day);
        sscanf(start_time, "%d %d", &event.date_start.hour, &event.date_start.minute);
        sscanf(end_date, "%d %d %d", &event.date_end.year, &event.date_end.month, &event.date_end.day);
        sscanf(end_time, "%d %d", &event.date_end.hour, &event.date_end.minute);
        event.reminder = atoi(reminder);
        strncpy(event.details, details, sizeof(event.details));

        // D-day 계산
        calDday(&event, current);

        // 기타 값 초기화 (일반 일정 기본값 설정)
        event.importance = -1; 
        event.quantity = -1;
        event.interval = -1;
        event.weight = -1;

        // Event 저장
        event.id = ++last_event_id;
        events[event_count++] = event;

        popup_message("Event successfully added!"); // 성공 메시지 출력
    } else {
        popup_message("Event creation canceled."); // 취소 메시지 출력
    }

	// 5. UI 화면 초기화
    active_screen = NULL;
}

// 일정 수정 (삭제 후 추가)
void modifyEvents() {
    if (event_count == 0) {
        popup_message("No events to modify!");
        return;
    }

	current_screen = DEFAULT_SCREEN;

    Time current;
    initTime(&current); // 현재 시간 초기화

    // 1. 현재 이벤트 리스트 출력
    clear();
    mvprintw(2, 10, "Modify Event:");
    mvprintw(4, 12, "Select an event to modify:");

    for (int i = 0; i < event_count; i++) {
        mvprintw(6 + i, 12, "%d. %s (Start: %02d-%02d | End: %02d-%02d)", 
                 i + 1, events[i].title,
                 events[i].date_start.month, events[i].date_start.day,
                 events[i].date_end.month, events[i].date_end.day);
    }

    // 2. 사용자 입력 처리
    char buffer[128] = {0};
    mvprintw(7 + event_count, 12, "Enter event number: ");
    if (get_input(buffer, sizeof(buffer)) == -1) {
        return; // 뒤로가기 처리
    }

    int choice = atoi(buffer);
    if (choice < 1 || choice > event_count) {
        popup_message("Invalid choice! Please try again.");
        return;
    }

    Event *event = &events[choice - 1];

	current_screen = EVENT_SCREEN;

    // 3. 입력 필드 정의
    char title[50] = {0}, details[100] = {0}, start_date[20] = {0}, start_time[10] = {0};
    char end_date[20] = {0}, end_time[10] = {0}, importance[5] = {0}, reminder[5] = {0};

    snprintf(title, sizeof(title), "%s", event->title);
    snprintf(details, sizeof(details), "%s", event->details);
    snprintf(start_date, sizeof(start_date), "%04d %02d %02d", 
             event->date_start.year, event->date_start.month, event->date_start.day);
    snprintf(start_time, sizeof(start_time), "%02d %02d", 
             event->date_start.hour, event->date_start.minute);
    snprintf(end_date, sizeof(end_date), "%04d %02d %02d", 
             event->date_end.year, event->date_end.month, event->date_end.day);
    snprintf(end_time, sizeof(end_time), "%02d %02d", 
             event->date_end.hour, event->date_end.minute);
    snprintf(importance, sizeof(importance), "%d", event->importance);
    snprintf(reminder, sizeof(reminder), "%d", event->reminder);

    InputField fields[] = {
        {"Modify title", title, sizeof(title), NULL, 1},
        {"Modify start date (YYYY MM DD)", start_date, sizeof(start_date), validate_date_wrapper, 1},
        {"Modify start time (HH MM)", start_time, sizeof(start_time), validate_time_wrapper, 1},
        {"Modify end date (YYYY MM DD)", end_date, sizeof(end_date), validate_date_wrapper, 1},
        {"Modify end time (HH MM)", end_time, sizeof(end_time), validate_time_wrapper, 1},
        {"Modify importance (range: 0-5)", importance, sizeof(importance), validate_importance, 1},
        {"Modify reminder (1: Yes, 0: No)", reminder, sizeof(reminder), validate_reminder, 1},
        {"Modify details", details, sizeof(details), NULL, 1}
    };

    UIScreen screen = {"Modify Event", fields, sizeof(fields) / sizeof(fields[0])};

    // 4. 입력 처리
    if (process_user_input(&screen) == 0) {
        // 5. 수정 데이터 반영
        strncpy(event->title, title, sizeof(event->title));
        sscanf(start_date, "%d %d %d", &event->date_start.year, &event->date_start.month, &event->date_start.day);
        sscanf(start_time, "%d %d", &event->date_start.hour, &event->date_start.minute);
        sscanf(end_date, "%d %d %d", &event->date_end.year, &event->date_end.month, &event->date_end.day);
        sscanf(end_time, "%d %d", &event->date_end.hour, &event->date_end.minute);
        event->importance = atoi(importance);
        event->reminder = atoi(reminder);
        strncpy(event->details, details, sizeof(event->details));

        // D-day 재계산
        calDday(event, current);

        popup_message("Event successfully modified!");
    } else {
        popup_message("Modification canceled.");
    }

	// 5. UI 화면 초기화
    active_screen = NULL;
}


// 일정 삭제
void deleteEvents() {
    if (event_count == 0) {
        popup_message("No events to delete!");
        return;
    }

	current_screen = DEFAULT_SCREEN;

    // 1. 현재 이벤트 목록 출력
    clear();
    mvprintw(2, 10, "Delete Event:");
    mvprintw(4, 10, "Select an event to delete:");

    for (int i = 0; i < event_count; i++) {
        mvprintw(6 + i, 10, "%d. %s (Start: %04d-%02d-%02d %02d:%02d)", 
                 i + 1, events[i].title,
                 events[i].date_start.year, events[i].date_start.month, events[i].date_start.day,
                 events[i].date_start.hour, events[i].date_start.minute);
    }

    mvprintw(7 + event_count, 10, "Enter the number of the event (or :b to go back): ");
    refresh();

    // 2. 사용자 입력 처리
    char buffer[128] = {0};
    if (get_input(buffer, sizeof(buffer)) == -1) {
        return; // 뒤로가기 처리
    }

    int choice = atoi(buffer);
    if (choice < 1 || choice > event_count) {
        popup_message("Invalid choice. Please try again.");
        return;
    }

    Event *event = &events[choice - 1];

    // 3. 삭제 확인 팝업 메시지 준비
    char confirmation[128] = "Delete \"";
    strcat(confirmation, event->title);
    strcat(confirmation, "\"? (y/n):");

    // 팝업 확인
    int confirm = popup_confirmation(confirmation);
    if (confirm == 0) { // 취소
        popup_message("Deletion canceled.");
        return;
    }

    // 4. 이벤트 삭제
    for (int i = choice - 1; i < event_count - 1; i++) {
        events[i] = events[i + 1]; // 배열 재정렬
    }
    event_count--; // 이벤트 개수 감소

    // 삭제된 이벤트 메모�� 초기화
    memset(&events[event_count], 0, sizeof(Event));

    popup_message("Event successfully deleted!");

	// 5. UI 화면 초기화
    active_screen = NULL;
}
