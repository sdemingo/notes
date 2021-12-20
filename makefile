all:
	gcc  -Wall notes.c -o notes -lform -lncurses
clean:
	rm notes
