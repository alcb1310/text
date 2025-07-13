#ifndef FILE_H_
#define FILE_H_

#include "typedefs.h"

char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();

#endif // !#ifndef FILE_H_
