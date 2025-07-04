/*** includes ***/

#include "defines.h"
#include "input.h"
#include "output.h"
#include "terminal.h"

/*** init ***/

void initEditor() {
  E.cx = 1;
  E.cy = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize");
}

int main() {
  enableRawMode();
  initEditor();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
