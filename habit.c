#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "habit.h"
#include "global.h"
#include "util.h"
#include "display.h"

time_t customParseDate(const char *date) {
    struct tm tm_info = {0};

    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return (time_t)-1;
    }

    char year[5], month[3], day[3];
    strncpy(year, date, 4); year[4] = '\0';
    strncpy(month, date + 5, 2); month[2] = '\0';
    strncpy(day, date + 8, 2); day[2] = '\0';

    tm_info.tm_year = atoi(year) - 1900;
    tm_info.tm_mon = atoi(month) - 1;
    tm_info.tm_mday = atoi(day);

    time_t result = mktime(&tm_info);
    if (result == -1) {
        fprintf(stderr, "Failed to parse date: %s\n", date);
    }
    return result;
}

// 현재 날짜를 가져오기
void getCurrentDate(char *date) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(date, 11, "%Y-%m-%d", tm_info);
}

void saveHabits() {
    FILE *file = fopen(HABIT_FILE, "w");
    if (!file) {
        printf("파일을 저장할 수 없습니다.\n");
        return;
    }

    // 첫 줄: 마지막 확인 날짜
    fprintf(file, "%s\n", last_checked_date);

    // 나머지 
    for (int i = 0; i < habit_count; i++) {
        fprintf(file, "%s %d %d\n", habits[i].name, habits[i].streak, habits[i].is_done);
    }
    fclose(file);
}

// 매일 전날과 비교해서 streak과 is_done 값을 초기화
void resetHabitsIfDateChanged() {
    char current_date[11];
    getCurrentDate(current_date);

    // 날짜 차이 계산
    time_t last_time = customParseDate(last_checked_date);
    time_t current_time = customParseDate(current_date);
    double diff_streak = difftime(current_time, last_time) / (60 * 60 * 24);

    
    if (diff_streak >= 1) {
        for (int i = 0; i < habit_count; i++) {
            if (diff_streak >= 2) {
                habits[i].streak = 0;
            }
            if(habits[i].is_done==0) { habits[i].streak = 0;}
            
            habits[i].is_done = 0; 
        }
        strcpy(last_checked_date, current_date);

    }
}

void signalresetHabits(){
    char current_date[11];
    getCurrentDate(current_date);
    
    for (int i = 0; i < habit_count; i++) {
            if(habits[i].is_done==0) habits[i].streak = 0;

            habits[i].is_done = 0; // 모든 과제 미완료 상태로 초기화
        }
        strcpy(last_checked_date, current_date);

        saveHabits();
}

// 파일에서 데이터 불러오기
void loadHabits() {
    FILE *file = fopen(HABIT_FILE, "r");
    if (!file) {
        // 파일이 없으면 새 파일 생성
        file = fopen(HABIT_FILE, "w");
        if (!file) {
            perror("파일 생성 실패");
            return;
        }
        getCurrentDate(last_checked_date); 
        fprintf(file, "%s\n", last_checked_date);
        fclose(file);

        habit_count = 0; 
        return;
    }
    

    if (fscanf(file, "%10s", last_checked_date) != 1) {
        getCurrentDate(last_checked_date); 

    }

    while (fscanf(file, "%49s %d %d", habits[habit_count].name, &habits[habit_count].streak, &habits[habit_count].is_done) == 3) {
        habit_count++;
        if (habit_count >= MAX_HABITS) break;
    }
    fclose(file);

    resetHabitsIfDateChanged();
}

// 습관 추가 함수
void add_habit() {
    if (habit_count >= MAX_HABITS) {
        popup_message("Habit list is full. Cannot add more habits.");
        return;
    }

    // 1. 입력 필드 준비
    char name[50] = {0};

    InputField fields[] = {
        {"Enter habit name(no spaces)", name, sizeof(name), NULL, 0} // 습관 이름은 유효성 검사 없음
    };

    UIScreen screen = {"Add Habit", fields, sizeof(fields) / sizeof(fields[0])};

    // 2. 화면 및 사용자 입력 처리
    active_screen = &screen;
    current_step = 0;

    if (process_user_input(&screen) == 0) {
        
        // 3. 공백 검사
        if (strchr(name, ' ') != NULL) {
            popup_message("Spaces not allowed Saving failed.");
            return;
        }
        // 3. 중복 체크
        for (int i = 0; i < habit_count; i++) {
            if (strcmp(habits[i].name, name) == 0) {
                popup_message("Habit is already stored!");
                return;
            }
        }

        // 4. 새로운 습관 저장
        Habit new_habit;
        new_habit.id = ++last_habit_id;
        strncpy(new_habit.name, name, sizeof(new_habit.name));
        new_habit.streak = 0;
        new_habit.is_done = 0;

        habits[habit_count++] = new_habit;

        popup_message("Habit successfully added!");
    } else {
        popup_message("Habit addition canceled.");
    }

    // 5. UI 화면 초기화
    active_screen = NULL;
}


// 습관 변경 함수
void change_habit() {
    if (habit_count == 0) {
        popup_message("No habits to change!");
        return;
    }

	current_screen = DEFAULT_SCREEN;

    // 1. 현재 존재하는 습관 목록을 출력
    clear();
    mvprintw(2, 10, "Change Habit:");
    mvprintw(4, 10, "Select a habit to change:");

    for (int i = 0; i < habit_count; i++) {
        mvprintw(6 + i, 10, "%d. %s (Streak: %d)", i + 1, habits[i].name, habits[i].streak);
    }

    mvprintw(7 + habit_count, 10, "Enter the number of the habit (or :b to go back):");
    refresh();

    // 2. 사용자 입력 처리
    char choice_buffer[128] = {0};
    if (get_input(choice_buffer, sizeof(choice_buffer)) == -1) {
        return; // 뒤로가기 처리
    }

    int choice = atoi(choice_buffer);
    if (choice < 1 || choice > habit_count) {
        popup_message("Invalid choice. Please try again.");
        return;
    }

    Habit *habit = &habits[choice - 1];

	current_screen = HABIT_SCREEN;
    
	// 3. 수정 UI 준비
    char new_name[50] = {0};
    snprintf(new_name, sizeof(new_name), "%s", habit->name);

    InputField fields[] = {
        {"Modify habit name", new_name, sizeof(new_name), NULL, 1}
    };

    UIScreen modify_screen = {
        "Change Habit",
        fields,
        sizeof(fields) / sizeof(fields[0])
    };

    // 4. 공통 입력 처리
    active_screen = &modify_screen; // 현재 UI 화면 설정
    current_step = 0;

    if (process_user_input(&modify_screen) == 0) {
        // 5. 공백 검사
        if (strchr(new_name, ' ') != NULL) {
            popup_message("Spaces not allowed. Change failed.");
            return;
        }
        // 5. 수정 데이터 반영
        strncpy(habit->name, new_name, sizeof(habit->name));
        popup_message("Habit successfully changed!");
    } else {
        popup_message("Change canceled.");
    }

    // 5. UI 화면 초기화
    active_screen = NULL;
}

// 습관 삭제 함수
void delete_habit() {
    if (habit_count == 0) {
        popup_message("No habits to delete!");
        return;
    }

    // 1. 현재 존재하는 습관 목록 출력
    clear();
    mvprintw(2, 10, "Delete Habit:");
    mvprintw(4, 10, "Select a habit to delete:");

    for (int i = 0; i < habit_count; i++) {
        mvprintw(6 + i, 10, "%d. %s (Streak: %d)", i + 1, habits[i].name, habits[i].streak);
    }

    mvprintw(7 + habit_count, 10, "Enter the number of the habit (or :b to go back):");
    refresh();

    // 2. 사용자 입력 처리
    char choice_buffer[128] = {0};
    if (get_input(choice_buffer, sizeof(choice_buffer)) == -1) {
        return; // 뒤로가기 처리
    }

    int choice = atoi(choice_buffer);
    if (choice < 1 || choice > habit_count) {
        popup_message("Invalid choice. Please try again.");
        return;
    }

    Habit *habit = &habits[choice - 1];

    // 3. 삭제 확인 메시지 출력
    char confirmation[128];
    snprintf(confirmation, sizeof(confirmation), "Delete \"%s\"? (y/n):", habit->name);

    int confirm = popup_confirmation(confirmation);  // 사용자 확인 요청
    if (confirm == 1) {
        // 4. 습관 삭제 처리
        for (int i = choice - 1; i < habit_count - 1; i++) {
            habits[i] = habits[i + 1];
        }
        habit_count--;

        popup_message("Habit successfully deleted!");
    } else {
        popup_message("Deletion canceled.");
    }

	// 5. UI 화면 초기화
    active_screen = NULL;
}

// 습관 성공 여부 입력
void mark_habit_success() {
    if (habit_count == 0) {
        popup_message("No habits to mark as success!");
        return;
    }

    // 1. 현재 습관 목록 출력
    clear();
    mvprintw(2, 10, "Mark Habit Success:");
    mvprintw(4, 10, "Select a habit to mark as success:");

    for (int i = 0; i < habit_count; i++) {
        mvprintw(6 + i, 10, "%d. %s (Streak: %d)", i + 1, habits[i].name, habits[i].streak);
    }

    mvprintw(7 + habit_count, 10, "Enter the number of the habit (or :b to go back): ");
    refresh();

    // 2. 사용자 입력 처리
    char buffer[128] = {0};
    if (get_input(buffer, sizeof(buffer)) == -1) {
        return; // 뒤로가기 처리
    }

    int choice = atoi(buffer);
    if (choice < 1 || choice > habit_count) {
        popup_message("Invalid choice. Please try again.");
        return;
    }

    Habit *habit = &habits[choice - 1];

    // 3. 성공 처리 확인
    if (habit->is_done) {
        popup_message("Habit already marked as success!");
    } else {
        habit->streak++;
        habit->is_done = 1;
        popup_message("Habit marked as success!");
    }

	// 5. UI 화면 초기화
    active_screen = NULL;
}

// 습관 관리 서브 메뉴
void habit_submenu() {
	current_screen = HABIT_SCREEN;

	int choice;
    
	while (1) {
        draw_habit_screen();
        choice = getch();

        if (choice == '1') {
            add_habit();
			clear();
			
			current_screen = HABIT_SCREEN;
			draw_habit_screen();
        } else if (choice == '2') {
            change_habit();
			clear();
			
			current_screen = HABIT_SCREEN;
			draw_habit_screen();
        } else if (choice == '3') {
			current_screen = DEFAULT_SCREEN;
            delete_habit();
			clear();
			
			current_screen = HABIT_SCREEN;
			draw_habit_screen();
        } else if (choice == '4') {
			current_screen = DEFAULT_SCREEN;
            mark_habit_success();
			clear();
			
			current_screen = HABIT_SCREEN;
			draw_habit_screen();
        } else if (choice == '5') {
            saveHabits();
            break;
        }
    }
}
