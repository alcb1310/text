#ifndef APPEND_H_
#define APPEND_H_

#include "typedefs.h"

void abAppend(struct abuf *ab, char *s, int len);
void abFree(struct abuf *ab);

#endif // !#ifndef APPEND_H_
