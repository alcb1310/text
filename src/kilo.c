/*** includes ***/

// this macros will tell the includes what features to expose
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "file.h"
#include "init.h"
#include "input.h"
#include "output.h"
#include "terminal.h"
#include "typedefs.h"

struct editorConfig E;

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
