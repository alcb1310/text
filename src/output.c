#include "output.h"

/*** output ***/

void editorDrawRows(struct abuf *ab) {
  for (int y = 0; y < E.screenrows; y++) {
    abAppend(ab, " ~", 2);

    if (y < E.screenrows - 1) {
      abAppend(ab, "\r\n", 2);
    }
  }
}

void editorRefreshScreen() {
  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[2J", 4);
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);

  abAppend(&ab, "\x1b[H", 3);
  write(STDOUT_FILENO, ab.b, ab.len);

  abFree(&ab);
}
