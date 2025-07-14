#include "row.h"
#include "syntax.h"

/***
 * Converts char index into render index
 *
 * @param *row Pointer to the row to convert
 * @param cx The char index
 */
int editorRowToRx(erow *row, int cx) {
  int rx = 0;
  for (int j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      rx += (KILO_TAB_STOPS - 1) - (rx % KILO_TAB_STOPS);
    }
    rx++;
  }
  return rx;
}

int editorRowRxToCx(erow *row, int rx) {
  int cur_rx = 0;

  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t') {
      cur_rx += (KILO_TAB_STOPS - 1) - (cur_rx % KILO_TAB_STOPS);
    }
    cur_rx++;

    if (cur_rx > rx) {
      return cx;
    }
  }

  return cx;
}

/***
 * Updates the row render with tabs
 *
 * @param row The row to update
 */
void editorUpdateRow(erow *row) {
  int tabs = 0;

  for (int j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      tabs++;
    }
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (KILO_TAB_STOPS - 1) + 1);

  int idx = 0;
  for (int j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      do {
        row->render[idx++] = ' ';
      } while (idx % KILO_TAB_STOPS != 0);
      continue;
    }
    row->render[idx++] = row->chars[j];
  }

  row->render[idx] = '\0';
  row->rsize = idx;

  editorUpdateSyntax(row);
}

/***
 * Appends a new row to the end of the row array
 */
void editorInsertRow(int at, char *s, size_t len) {
  if (at < 0 || at > E.numrows) {
    return;
  }

  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
	for (int j = at + 1; j <= E.numrows; j++) {
		E.row[j].idx++;
	}

	E.row[at].idx = at;

  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  E.row[at].hl = NULL;
	E.row[at].hl_open_comment = 0;
  editorUpdateRow(&E.row[at]);

  E.numrows++;
  E.dirty++;
}

/***
 * Free the row memory allocated
 *
 * @param *row The row to free
 */
void editorFreeRow(erow *row) {
  free(row->render);
  free(row->chars);
  free(row->hl);
}

/***
 * Deletes a row from the row array
 *
 * @param at The index of the row to delete
 */
void editorDelRow(int at) {
  if (at < 0 || at >= E.numrows) {
    // There is no row to delete
    return;
  }

  editorFreeRow(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));

	for (int j = at; j < E.numrows -1; j++) {
		E.row[j].idx--;
	}

  E.numrows--;
  E.dirty++;
}

/***
 * Inserts a character into the current row
 *
 * @param *row The row to insert into
 * @param at The index to insert at
 * @param c The character to insert
 */
void editorRowInsertChar(erow *row, int at, int c) {
  if (at < 0 || at > row->size) {
    at = row->size;
  }

  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  editorUpdateRow(row);
  E.dirty++;
}

/***
 * Appends a string to the current row
 *
 * @param *row The row to append to
 * @param s The string to append
 * @param len The length of the string to append
 */
void editorRowAppendString(erow *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  editorUpdateRow(row);
  E.dirty++;
}

/***
 * Deletes a character at the given positon
 *
 * @param *row The row to delete from
 * @param at The index to delete
 */
void editorRowDelChar(erow *row, int at) {
  if (at < 0 || at >= row->size) {
    return;
  }

  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}
