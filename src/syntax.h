#ifndef SYNTAX_H_
#define SYNTAX_H_

#include "typedefs.h"

int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight();

#endif // !#ifndef SYNTAX_H_
