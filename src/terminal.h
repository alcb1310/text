#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "typedefs.h"

void die(const char *s);
void disableRawMode();
void enableRowMode();
int editorReadKey();
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);

#endif // !#ifndef TERMINAL_H_
