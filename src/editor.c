#include "editor.h"
#include "row.h"
#include "typedefs.h"

/***
 * Inserts a character into the current row
 *
 * @param c The character to insert
 */
void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    editorInsertRow(E.numrows, "", 0);
  }

  editorRowInsertChar(&E.row[E.cy], E.cx - KILO_SIGN_COLUMN, c);
  E.cx++;
}

/***
 * Inserts a new line
 */
void editorInsertNewLine() {
  if (E.cx == KILO_SIGN_COLUMN) {
    editorInsertRow(E.cy, "", 0);
  } else {
    erow *row = &E.row[E.cy];
    editorInsertRow(E.cy + 1, &row->chars[E.cx - KILO_SIGN_COLUMN],
                    row->size - E.cx + KILO_SIGN_COLUMN);
    row = &E.row[E.cy];
    row->size = E.cx - KILO_SIGN_COLUMN;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }

  E.cy++;
  E.cx = KILO_SIGN_COLUMN;
}

/***
 * Deletes a character from the current row
 */
void editorDelChar() {
  if (E.cy == E.numrows) {
    return;
  }
  if (E.cx == KILO_SIGN_COLUMN && E.cy == 0) {
    return;
  }

  erow *row = &E.row[E.cy];
  if (E.cx > KILO_SIGN_COLUMN) {
    editorRowDelChar(row, E.cx - KILO_SIGN_COLUMN - 1);
    E.cx--;
  } else {
    E.cx = E.row[E.cy - 1].size + KILO_SIGN_COLUMN;
    editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
    editorDelRow(E.cy);
    E.cy--;
  }
}
