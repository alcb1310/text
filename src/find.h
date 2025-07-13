#ifndef FIND_H_
#define FIND_H_

#include "typedefs.h"

void editorFindCallback(char *query, int key);
void editorFind();

// TODO: remove this functions when refactor is done
char *editorPrompt(char *prompt, void (*callback)(char *, int));

#endif // !#ifndef FIND_H_
