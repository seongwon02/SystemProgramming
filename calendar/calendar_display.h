#ifndef CALENDAR_DISPLAY_H
#define CALENDAR_DISPLAY_H

#include <sys/ioctl.h>

void show_big_month(int year, int month, int day, struct winsize *wbuf); // 달력 형태의 달 기준 캘린더(큰 화면)
void show_small_month(int year, int month, struct winsize *wbuf);		 // 표 형태의 달 기준 캘린더(작은 화면)
void show_week(int year, int month, int day, struct winsize *wbuf);		 // 주 기준 캘린더 
void show_day(int year, int month, int day, struct winsize *wbuf);		 // 일 기준 캘린더

#endif
