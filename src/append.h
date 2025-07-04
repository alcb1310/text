#ifndef APPEND_H
#define APPEND_H

#include "defines.h"
#include <string.h>

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif
