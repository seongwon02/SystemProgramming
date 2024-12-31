CC = gcc
CFLAGS = -Wall
LDFLAGS = -lncurses -lpthread

TARGET = project
CALENDAR = ./calendar/calendar.o ./calendar/calendar_control.o ./calendar/calendar_display.o ./calendar/calendar_get_event.o
OBJS = main.o \
       display.o \
       event.o \
       habit.o \
       scheduler.o \
       util.o \
       date_check.o \
       global.o \
       ${CALENDAR}

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# integration 디렉토리의 .c 파일 규칙
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# calendar 디렉토리의 .c 파일 규칙
calendar/%.o: calendar/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET) *.o calendar/*.o

