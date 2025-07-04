#include "input.h"

/*** input ***/

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(EXIT_SUCCESS);
    break;

  case 'k':
  case 'j':
  case 'h':
  case 'l':
    editorMoveCursor(c);
    break;
  }
}

void editorMoveCursor(char key) {
  switch (key) {
  case 'h':
    if (E.cx != 0)
      E.cx--;
    break;
  case 'l':
    if (E.cx != E.screencols - 1)
      E.cx++;
    break;
  case 'k':
    if (E.cy != 0)
      E.cy--;
    break;
  case 'j':
    if (E.cy != E.screenrows - 1)
      E.cy++;
    break;
  }
}
