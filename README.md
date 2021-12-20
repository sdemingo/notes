# Notes

Aplicación para gestionar anotaciones sobre el terminal construida sobre la librería `ncurses`. La uso para manejar los archivos sobre notas y anotaciones que guardo en mi servidor a través de un bot de telegram o bien desde el correo electrónico. Manejo una nota por archivo en formato texto, todas ellas en un mismo directorio. Por eso mismo la aplicación no navega entre directorios.

Para navegar puedes usar las teclas:

- `Arriba/Abajo`: Navegar entre notas.
- `Enter`: Mostrar el contenido de una nota
- `d`: Borrar una nota (realmente la pasa a /tmp)
- `:`: Entra en modo búsqueda. Permite escribir las etiquetas para filtrar las notas
- `q`: Salir


## Instalación

Para personalizar la aplicación no uso ficheros de configuración, asi que tendrás que modificar las variables como `editor` o `dirnotes` para indicar el directorio con el que quieras trabajar. Para compilar (en Debian) necesitas tener instalados los paquetes `libncurses5-dev`y `libncursesw5-dev` y luego ejecutar el comando `make`.
