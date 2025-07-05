#ifndef FILE_H
#define FILE_H

#include "defines.h"
#include "string.h"
#include <stdio.h>

void editorOpen(char *filename);
void editorAppendRow(char *s, size_t len);

#endif // !FILE_H
