# all:
# 	gcc  notes.c -o notes -lform -lncurses -Wall 
# clean:
# 	rm notes


OBJS = notes.o utils.o
HDR = utils.h
FLAGS = -lform -lncurses -Wall 

%.o: %.c
	gcc $(FLAGS) -g -c $< -o $@


notes: $(OBJS)
	gcc $(OBJS) -o notes -lform -lncurses -Wall 


all: notes

clean:
	rm $(OBJS)