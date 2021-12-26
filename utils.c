#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>




// Cara el directorio en la lista de navegación
int get_files(char *dir, char ***list)
{
  DIR *dirp;
  struct dirent *direntp;
  size_t n = 0;

  dirp = opendir(dir);
  if (dirp == NULL)
  {
    printf("Error: No se puede abrir el directorio de notas: %s\n", dir);
    exit(0);
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



// Filtrado de notas por etiqueta. Retorna los elementos filtrados
int filter_by_tag(char *dir, char ***list, int nlist, char *tag)
{
  int i, l;
  FILE *fp;
  char *notepath;
  char line[300];
  char **filtered = NULL;
  int n_filtered = 0;
  int len;

  for (i = 0; i < nlist; i++)
  {
    len = strlen(dir) + strlen((*list)[i]) + 2;
    notepath = malloc(len * sizeof(char));
    snprintf(notepath,len,"%s/%s",dir,(*list)[i]);

    fp = fopen(notepath, "r");
    if (fp != NULL)
    {
      for (l = 0; l < 2; l++) // only reads two first lines
      {
        fgets(line, 300, fp);
        if (strstr(line, tag) != NULL)
        {
          //  Hemos encontrado una nota con esa etiqueta
          filtered = realloc(filtered, sizeof(filtered) * (n_filtered + 1));
          filtered[n_filtered++] = strdup((*list)[i]);
          free((*list)[i]);
        }
      }
    }
    free(notepath);
  }

  // hago el cambio
  free(*list);
  *list = filtered;

  return n_filtered;
}

// Borrado de nota
void delete_file(char *dir, char ***list, int number)
{
  char *fname = (*list)[number];
  char *tmpdir = "/tmp";
  char *oldpath = malloc((strlen(dir) + strlen(fname) + 2) * sizeof(char));
  char *newpath = malloc((strlen(tmpdir) + strlen(fname) + 2) * sizeof(char));

  strcat(oldpath, dir);
  strcat(oldpath, "/");
  strcat(oldpath, fname);

  strcat(newpath, tmpdir);
  strcat(newpath, "/");
  strcat(newpath, fname);

  rename(oldpath, newpath);

  free(oldpath);
  free(newpath);

  //files_deleted = true;
}

