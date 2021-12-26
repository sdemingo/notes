#include "utils.h"


int main()
{
  highlight = 0;
  mode = MODE_NAV;
  bool exit = false;
  int c;

  n_tag_chars = 0;
  files_deleted = false;
  char *homedir = getenv("HOME");
  dirname = malloc((strlen(homedir) + strlen(dirnotes) + 2) * sizeof(char));
  strcat(dirname, homedir);
  strcat(dirname, "/");
  strcat(dirname, dirnotes);

  signal(SIGWINCH, sig_winch);

  choices = NULL;
  n_choices = get_files(dirname, &choices);

  initscr();
  clear();
  noecho();
  cbreak(); /* Line buffering disabled. pass on everything */

  menu_win = newwin(LINES - 4, COLS, 3, 0);

  keypad(menu_win, TRUE);

  /*  Imprimimos la barra de título  */
  int t = 0;
  attron(A_REVERSE | A_BOLD);
  for (t = 0; t < COLS; t++)
  {
    mvprintw(0, t, " ");
  }
  mvprintw(0, 5, "%s", TITLE);
  attroff(A_REVERSE | A_BOLD);
  refresh();

  print_info_bar();
  print_tag_buffer();
  print_list(menu_win, highlight);

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
      if ((mode == MODE_NAV) && (highlight < n_choices - 1))
        ++highlight;
      break;

    case KEY_BACKSPACE:
      if (mode == MODE_TAG)
        if (n_tag_chars > 0)
          tag_buffer[--n_tag_chars] = '\0';
      break;

    case ':':
      if (mode == MODE_NAV)
        mode = MODE_TAG;
      else
        mode = MODE_NAV;
      break;

    case 10: // Enter
      if ((mode == MODE_NAV) && (n_choices>0))
        print_file(menu_win, highlight);

      if (mode == MODE_TAG)
      {
        if (strlen(tag_buffer) != 0)
        {
          n_choices = filter_by_tag(dirname, &choices, n_choices, tag_buffer); // actualizo lista
          print_tag_buffer(0);
        }
        else
        {
          choices = NULL;  // actualizo lista
          n_choices = get_files(dirname, &choices);
          highlight = 0;
        }
        mode = MODE_NAV;
      }
      break;

    case 'q':
      if (mode == MODE_NAV)
        exit = true;
      if (mode == MODE_TAG)
        tag_buffer[n_tag_chars++] = c;
      break;

    case 'd':
      if (mode == MODE_NAV){
        delete_file(dirname, highlight);
        choices = NULL; // actualizo lista
        n_choices = get_files(dirname, &choices);
        files_deleted = true;
      }
      if (mode == MODE_TAG)
        tag_buffer[n_tag_chars++] = c;
      break;

    case 'e':
      if (mode == MODE_NAV)
        edit_file(menu_win, highlight);

      if (mode == MODE_TAG)
        tag_buffer[n_tag_chars++] = c;
      break;

    default:
      if (mode == MODE_TAG)
        tag_buffer[n_tag_chars++] = c;
      break;
    }

    print_list(menu_win, highlight);
    print_tag_buffer();
  }

  wclear(menu_win);
  clear();

  clrtoeol();
  refresh();
  endwin();

  free(dirname);
  if (files_deleted)
    printf("AVISO:\nAlgunas notas fueron marcadas para eliminar y movidas al directorio temporal\n\n");

  return 0;
}








// Manejador de eventos para la interrupción de redimensionado de la terminal
void sig_winch(int in)
{
  endwin();
  refresh();
  wclear(menu_win);
  resizeterm(LINES, COLS);

  wresize(menu_win, LINES - 4, COLS);
  print_list(menu_win, highlight);
}






// Muestra la lista de archivos o notas en la parte izquierda de la pantalla
// y permirte navegar entre ellas. El limite de las notas mostradas lo pone
// la variable choices_showed
void print_list(WINDOW *win, int highlight)
{
  int x, y, i;
  int from, to;
  x = 1;
  y = 1;

  box(win, 0, 0);
  from = highlight;
  to = highlight + choices_showed;

  for (i = from; i < to; ++i)
  {
    if ((i >= 0) && (i < n_choices))
    {
      if (i == from)
      {
        if (mode == MODE_NAV)
        {
          wattron(win, A_REVERSE);
          mvwprintw(win, y, x, "%s", choices[i]);
          wattroff(win, A_REVERSE);
        }
        else
        {
          mvwprintw(win, y, x, "%s", choices[i]);
        }
      }
      else
      {
        mvwprintw(win, y, x, "%s", choices[i]);
      }
    }
    else
    {
      mvwprintw(win, y, x, "             ");
    }

    ++y;
  }

  wmove(win, 1, NOTES_NAMES_WIDTH - 1);
  wvline(win, 0, LINES - 6);
  wrefresh(win);
}

// Muestra el fichero en la parte derecha de la pantalla
void print_file(WINDOW *win, int number)
{
  FILE *fp;
  char line[300];
  int line_max_len = COLS / 2;
  int line_off = 2;
  char *filename = NULL;

  int len = strlen(dirname) + strlen(choices[number]) + 2;
  filename = malloc(len * sizeof(char));
  snprintf(filename, len, "%s/%s", dirname, choices[number]);

  fp = fopen(filename, "r");
  if (fp != NULL)
  {
    while (fgets(line, 300, fp) != NULL)
    {
      line[line_max_len] = 0; // solo muestra la parte de la línea que entra en la ventana
      mvwprintw(menu_win, line_off, NOTES_NAMES_WIDTH, "%s", line);
      line_off++;
    }
  }

  free(filename);
  wrefresh(menu_win);
}

// Comando de edición. Lllamada al sistema para arrancar el editor indicado
void edit_file(WINDOW *win, int number)
{
  int len = strlen(editor) + strlen(dirname) + strlen(choices[number]) + 3;
  char *command = malloc(len * sizeof(char));
  snprintf(command, len, "%s %s/%s", editor, dirname, choices[number]);

  endwin();
  refresh();
  wclear(win);

  system(command);
  free(command);
}


// Muestra el buffer de filtrado de etiquetas
void print_tag_buffer()
{
  char *title = "Etiqueta:";
  move(2, 0);
  clrtoeol();
  if (mode == MODE_TAG)
  {
    attron(A_REVERSE);
    mvprintw(2, 0, title);
    attroff(A_REVERSE);
  }
  else
  {
    mvprintw(2, 0, title);
  }
  mvprintw(2, strlen(title) + 1, tag_buffer);
  refresh();
}

// Muestra la barra inferior
void print_info_bar()
{
  int u = 0;
  attron(A_REVERSE | A_BOLD);
  for (u = 0; u < COLS; u++)
  {
    mvprintw(LINES - 1, u, " ");
  }
  mvprintw(LINES - 1, 1, "%s", INFOBAR);
  attroff(A_REVERSE | A_BOLD);
  refresh();
}
