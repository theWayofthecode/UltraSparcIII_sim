
OBJ=cpu.o instruction_issue_unit.o integer_unit.o ncurses_ui.o
HDRS=cpu.h instruction_issue_unit.h integer_unit.h ncurses_ui.h
LIBS=-lncurses -lpthread
CFLAGS=-Wall -g

UltraSparcIII: ${OBJ}
	gcc ${CFLAGS} -o UltraSparcIII ${OBJ} ${LIBS}

cpu.o: cpu.c ${HDRS}
	gcc ${CFLAGS} -c cpu.c

instruction_issue_unit.o: instruction_issue_unit.c ${HDRS}
	gcc ${CFLAGS} -c instruction_issue_unit.c

integer_unit.o: integer_unit.c ${HDRS}
	gcc ${CFLAGS} -c integer_unit.c

ncurses_ui.o: ncurses_ui.c ncurses_ui.h
	gcc ${CFLAGS} -c ncurses_ui.c

clean:
	rm -f ${OBJ}
	rm -f UltraSparcIII
