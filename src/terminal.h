#ifndef TERMINAL_H
#define TERMINAL_H

#include "defines.h"

void disableRawMode();
void enableRawMode();
int editorReadKey();
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);

#endif
