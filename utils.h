#include <stdio.h>
#include <dirent.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


// resources from utils functions
const char *dirnotes = ".cairen/notes";
const char *editor = "emacs";

// resources from ui functions
const int NOTES_NAMES_WIDTH = 30;
const char *TITLE = "Notes 1.0";
const char *INFOBAR = " [q]=>Salir  [Enter]=>Mostrar  [e]=>Editar  [:]=>Filtrar  [d]=>Borrar";
const int choices_showed = 15;
const int MODE_NAV = 0;
const int MODE_TAG = 1;

// from all
WINDOW *menu_win;
int mode;
int highlight;

char *dirname;
char **choices;
int n_choices;
char tag_buffer[100];
int n_tag_chars;
bool files_deleted;

// ui functions
void print_list(WINDOW *win, int highlight);
void print_file(WINDOW *win, int highlight);
void print_info_bar();
void print_tag_buffer();

void sig_winch(int in);


// utils.c functions
void edit_file(WINDOW *win, int number);
void delete_file(char *dir,int number);
int get_files(char *dir,char ***list);
int filter_by_tag(char *dir,char ***list, int nlist, char *tag);
