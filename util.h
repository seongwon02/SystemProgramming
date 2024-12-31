// util.h
#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <ncurses.h>
#include <string.h>
#include "display.h"

//유효성 테스트용 임시
bool validate_title(const char *input);
bool validateDate(int year, int month, int day);
bool validateTime(int hour, int minute);
bool validate_date_wrapper(const char *input);
bool validate_time_wrapper(const char *input);
bool validate_reminder(const char *input);
bool validate_importance(const char *input);
bool validate_quantity(const char *input);
//validTest

void popup_message(const char *message);
int popup_confirmation(const char *message);
void reset_input_position();
void handle_resize(int sig);
int get_input(char *buffer, int size);
int process_user_input(UIScreen *screen);

//Sorting
int compareByWeight(const void* a, const void* b);
void sortTodo(Event* event_t, int count);

int isLeapYear(int year);
int daysInMonth(int month, int year);

extern bool popup_message_called;
extern UIScreen *active_screen;
extern int current_step;

#endif
