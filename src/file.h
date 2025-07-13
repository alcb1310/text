#ifndef FILE_H_
#define FILE_H_

#include "typedefs.h"

char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();

// TODO: remove this functions when refactor is done
void editorSetStatusMessage(const char *fmt, ...);
char *editorPrompt(char *prompt, void (*callback)(char *, int));

#endif // !#ifndef FILE_H_
