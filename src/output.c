#include "output.h"
#include "defines.h"
#include <unistd.h>

/*** output ***/

void editorDrawRows() {
  for (int y = 0; y < E.screenrows; y++) {
    write(STDOUT_FILENO, " ~", 2);

    if (y < E.screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}

void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  write(STDOUT_FILENO, "\x1b[H", 3);
}
