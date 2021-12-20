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

/*
  Cosas que faltan:
  - Agregar filtrado por etiquetas. 
  - Eliminar el uso de menu_win y hacerlo todo en la ventana estándar por simplificar
*/

const int NOTES_NAMES_WIDTH = 30;
const char *TITLE = "Notes 0.5";
const char *INFOBAR = " [q]=>Salir  [Enter]=>Mostrar  [e]=>Editar  [:]=>Filtrar  [d]=>Borrar";
const char *dirname = "~/.cairen/notes";
const char *editor = "emacs";
const int choices_showed = 15;

const int MODE_NAV = 0;
const int MODE_TAG = 1;

int startx = 0;
int starty = 0;

WINDOW *menu_win;
int mode;
int highlight;

char **choices;
int n_choices;
char tag_buffer[100];
int n_tag_chars=0;
bool files_deleted=false;

void print_menu(WINDOW *menu_win, int highlight);
void print_file(WINDOW *win, int highlight);
void print_info_bar();
void print_tag_buffer();

void edit_file(WINDOW *win, int number);
void delete_file(int number);
int get_notes_filenames(char ***list);
void sig_winch(int in);

void filter_by_tag();

int main()
{
  highlight = 0;
  mode = MODE_NAV;
  bool exit = false;
  int c;

  signal(SIGWINCH, sig_winch);
  n_choices = get_notes_filenames(&choices);

  initscr();
  clear();
  noecho();
  cbreak(); /* Line buffering disabled. pass on everything */
  startx = 0;
  starty = 3;

  menu_win = newwin(LINES - 4, COLS, starty, startx);

  keypad(menu_win, TRUE);

  /*  Imprimimos la barra de título  */
  int t=0;
  attron(A_REVERSE | A_BOLD);	
  for (t=0;t<COLS;t++){
    mvprintw(0, t, " ");
  }
  mvprintw(0, 5, "%s",TITLE);
  attroff(A_REVERSE | A_BOLD);
  refresh();
   
  print_info_bar(); 
  print_tag_buffer();
  print_menu(menu_win, highlight);
  
  while (!exit)
    {
      c = wgetch(menu_win);
      wclear(menu_win);
      switch (c)
        {
        case KEY_UP:
	  if ((mode == MODE_NAV) && (highlight > 0))
	    --highlight;
	  break;

        case KEY_DOWN:
	  if ((mode == MODE_NAV) && (highlight < n_choices-1))
	    ++highlight;
	  break;

	case KEY_BACKSPACE:
	  if (mode == MODE_TAG)
	    if (n_tag_chars > 0)
	      tag_buffer[--n_tag_chars]='\0';
	  break;
	      
	case ':':
	  if (mode == MODE_NAV)
	    mode=MODE_TAG;
	  else
	    mode=MODE_NAV;
	  break;
	  

	case 10:
	  if (mode==MODE_NAV)
	    print_file(menu_win, highlight);
	  
  	  if (mode==MODE_TAG){
	    filter_by_tag();
	    mode=MODE_NAV;
	    n_tag_chars=0;
	    tag_buffer[0]='\0';
	  }
	  break;

        case 'q':
	  if (mode==MODE_NAV)
	    exit = true;	  
	  if (mode==MODE_TAG)
	    tag_buffer[n_tag_chars++]=c;
	  break;

	case 'd':
	  if (mode==MODE_NAV)
	    delete_file(highlight);
	  if (mode==MODE_TAG)
	    tag_buffer[n_tag_chars++]=c;
	  break;
	  

        case 'e':
	  if (mode==MODE_NAV)
	    edit_file(menu_win, highlight);
	  
	  if (mode==MODE_TAG)
    	    tag_buffer[n_tag_chars++]=c;
	  break;

	default:
	  if (mode==MODE_TAG)
       	    tag_buffer[n_tag_chars++]=c;
	  break;
        }

      print_menu(menu_win, highlight);
      print_tag_buffer();


    }

  wclear(menu_win);
  clear();
    
  clrtoeol();
  refresh();
  endwin();

  if (files_deleted)
    printf("AVISO:\nAlgunas notas fueron marcadas para eliminar y movidas al directorio temporal\n\n");

  return 0;
}



// Muestra la lista de archivos o notas en la parte izquierda de la pantalla
// y permirte navegar entre ellas. El limite de las notas mostradas lo pone
// la variable choices_showed
void print_menu(WINDOW *menu_win, int highlight)
{
  int x, y, i;
  int from, to;
  x = 1;
  y = 1;
  
  box(menu_win, 0, 0);
  from = highlight;
  to = highlight + choices_showed;

  for (i = from; i < to; ++i)
    {
      if ((i >= 0) && (i < n_choices))
        {
	  if (i == from)
            {
	      if (mode==MODE_NAV){
		wattron(menu_win, A_REVERSE);
		mvwprintw(menu_win, y, x, "%s", choices[i]);
		wattroff(menu_win, A_REVERSE);
	      }else{
		mvwprintw(menu_win, y, x, "%s", choices[i]);
	      }
            }
	  else
            {
	      mvwprintw(menu_win, y, x, "%s", choices[i]);
            }
        }
      else
        {
	  mvwprintw(menu_win, y, x, "             ");
        }

      ++y;
    }

  wmove(menu_win,1,NOTES_NAMES_WIDTH-1);
  wvline(menu_win, 0, LINES - 6);
  wrefresh(menu_win);
}


// Muestra el fichero en la parte derecha de la pantalla
void print_file(WINDOW *win, int number)
{
  FILE *fp;
  char line[300];
  int line_max_len = COLS / 2;
  int line_off = 2;

  char *filename = malloc((strlen(dirname)
			   + strlen(choices[number]) + 2) * sizeof(char));
  strcat(filename,dirname);
  strcat(filename,"/");
  strcat(filename,choices[number]);
  
  fp = fopen(filename, "r");
  if (fp == NULL)
    {
      mvwprintw(menu_win, 2, NOTES_NAMES_WIDTH, "Fichero %s no pudo abrirse", filename);
      return;
    }

  while (fgets(line, 300, fp) != NULL)
    {
      line[line_max_len] = 0; // solo muestra la parte de la línea que entra en la ventana
      mvwprintw(menu_win, line_off, NOTES_NAMES_WIDTH, "%s", line);
      line_off++;
    }

  free(filename);
  wrefresh(menu_win);
}


// Comando de edición. Lllamada al sistema para arrancar el editor indicado
void edit_file(WINDOW *win, int number)
{
  char *command = malloc((strlen(editor)
			  + strlen(dirname)
			  + strlen(choices[number]) + 3) * sizeof(char));

  strcat(command,editor);
  strcat(command," ");
  strcat(command,dirname);
  strcat(command,"/");
  strcat(command,choices[number]);

  endwin();
  refresh();
  wclear(win);
    
  system(command);
  free(command);
}


// Cara el directorio en la lista de navegación
int get_notes_filenames(char ***list)
{
  DIR *dirp;
  struct dirent *direntp;
  size_t n = 0;

  dirp = opendir(dirname);
  if (dirp == NULL)
    {
      printf("Error: No se puede abrir el directorio de notas\n");
      return -1;
    }
  while ((direntp = readdir(dirp)) != NULL)
    {
      if (direntp->d_name[0] != '.')
        {
	  *list = realloc(*list, sizeof(**list) * (n + 1)); // incremento en uno el tamaño de list
	  (*list)[n++] = strdup(direntp->d_name);           // inserto una copia del nombre del fichero
        }
    }

  closedir(dirp);
  return n;
}


// Muestra el buffer de filtrado de etiquetas
void print_tag_buffer()
{
  char *title="Etiqueta:";
  move(2,0);
  clrtoeol();
  if (mode == MODE_TAG){
    attron(A_REVERSE);
    mvprintw(2,0,title);
    attroff(A_REVERSE);
  }else{
    mvprintw(2,0,title);
  }
  mvprintw(2,strlen(title)+1,tag_buffer);  
  refresh();
}


// Muestra la barra inferior
void print_info_bar()
{
  int u=0;
  attron(A_REVERSE | A_BOLD);	
  for (u=0;u<COLS;u++){
    mvprintw(LINES-1, u, " ");
  }
  mvprintw(LINES-1, 1, "%s",INFOBAR);
  attroff(A_REVERSE | A_BOLD);
  refresh();
}


// Filtrado de notas por etiqueta
void filter_by_tag()
{
  print_tag_buffer(0);
  /*
    Debo realizar el filtrado y recagar choices 
    solo con las adecuadas
  */
}

// Borrado de nota
void delete_file(int number)
{
  char *fname=choices[number];
  char *tmpdir="/tmp";
  char *oldpath=malloc((strlen(dirname)+strlen(fname)+2)*sizeof(char));
  char *newpath=malloc((strlen(tmpdir)+strlen(fname)+2)*sizeof(char));

  strcat(oldpath,dirname);
  strcat(oldpath,"/");
  strcat(oldpath,fname);

  strcat(newpath,tmpdir);
  strcat(newpath,"/");
  strcat(newpath,fname);

  rename(oldpath,newpath);

  free(oldpath);
  free(newpath);
  files_deleted=true;

  n_choices = get_notes_filenames(&choices);  
}



// Manejador de eventos para la interrupción de redimensionado de la terminal
void sig_winch(int in)
{
  endwin();
  refresh();
  wclear(menu_win);
  resizeterm(LINES, COLS);

  wresize(menu_win, LINES - 4, COLS);
  print_menu(menu_win, highlight);
}
