#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "typedefs.h"

void editorScroll();
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen();
void editorDrawSignColumn(struct abuf *ab, int numrow);

#endif // !#ifndef OUTPUT_H_
