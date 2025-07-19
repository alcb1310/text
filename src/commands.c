#include "commands.h"
#include "input.h"

void quit() {
  if (E.dirty) {
    editorSetStatusMessage(
        "WARNING! File has unsaved changes! :w to save or :q! "
        "to quit without saving");
    return;
  }
  force_quit();
}

void force_quit() {
  write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
  write(STDOUT_FILENO, "\x1b[H", 3);  // cursor home
  exit(EXIT_SUCCESS);
}
