#ifndef INPUT_H
#define INPUT_H

#include "typedefs.h"

char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
void editorNormalProcessKeypress(int c);
void editorInsertProcessKeypress(int c);
void editorProcessKeypress();
void editorSetStatusMessage(const char *fmt, ...);
void editorCommandMode();

#endif // !INPUT_H
