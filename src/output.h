#ifndef OUTPUT_H
#define OUTPUT_H

#include "append.h"
#include "defines.h"
#include <unistd.h>

void editorDrawRows(struct abuf *ab);
void editorRefreshScreen();

#endif
