/*** includes ***/

// this macros will tell the includes what features to expose
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "file.h"
#include "input.h"
#include "output.h"
#include "terminal.h"
#include "typedefs.h"

struct editorConfig E;

/*** init ***/

/***
 * Initializes the program
 */
void initEditor() {
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.dirty = 0;
  E.mode = NORMAL_MODE;
  E.row = NULL;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
    die("getWindowSize");
  }

  E.screenrows -= 2;
}

int main(int argc, char *argv[]) {
  enableRowMode();
  initEditor();
  if (argc >= 2) {
    editorOpen(argv[1]);
  }

  editorSetStatusMessage(DEFAULT_MESSAGE);

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
