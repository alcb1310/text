#include "input.h"
#include "defines.h"
#include "terminal.h"

/*** input ***/

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(EXIT_SUCCESS);
    break;
  }
}
