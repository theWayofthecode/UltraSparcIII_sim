
LIBS=-lncurses -lpthread
CFLAGS=-Wall -g
OBJ=cpu.o instruction_issue_unit.o ncurses_ui.o

UltraSparcIII: ${OBJ}
	gcc ${CFLAGS} -o UltraSparcIII ${OBJ} ${LIBS}

cpu.o: cpu.c
	gcc ${CFLAGS} -c cpu.c

instruction_issue_unit.o: instruction_issue_unit.c
	gcc ${CFLAGS} -c instruction_issue_unit.c

ncurses_ui.o: ncurses_ui.c
	gcc ${CFLAGS} -c ncurses_ui.c

clean:
	rm -f ${OBJ}
	rm -f UltraSparcIII
