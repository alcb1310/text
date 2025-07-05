/*** includes ***/

#include "defines.h"
#include "file.h"
#include "input.h"
#include "output.h"
#include "terminal.h"

/*** init ***/

void initEditor() {
  E.cx = 1;
  E.cy = 0;
  E.numrows = 0;
  E.row = NULL;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize");
}

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();
  if (argc >= 2) {
    editorOpen(argv[1]);
  }

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
