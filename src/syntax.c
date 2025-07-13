#include "syntax.h"

/***
 * define if a character is a separator
 *
 * @param c the character
 * @returns whether the character is a separator or not
 */
int is_separator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];\"", c) != NULL;
}

void editorUpdateSyntax(erow *row) {
  row->hl = realloc(row->hl, row->size);
  memset(row->hl, HL_NORMAL, row->size);

  int prev_sep = 1;

  int i = 0;
  while (i < row->rsize) {
    char c = row->render[i];
    unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

    if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
        (c == '.' && prev_hl == HL_NUMBER)) {
      row->hl[i] = HL_NUMBER;
      i++;
      prev_sep = 0;
      continue;
    }

    prev_sep = is_separator(c);
    i++;
  }
}

int editorSyntaxToColor(int hl) {
  switch (hl) {
  case HL_NUMBER:
    return 31;
  case HL_MATCH:
    return 34;
  default:
    return 37;
  }
}
