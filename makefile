all:
	gcc  notes.c -o notes -lform -lncurses -Wall 
clean:
	rm notes

