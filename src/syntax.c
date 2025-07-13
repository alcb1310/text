#include "syntax.h"

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
struct editorSyntax HLDB[] = {{
    "c",
    C_HL_extensions,
    HL_HIGHLIGHT_NUMBERS,
}};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

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

  if (E.syntax == NULL) {
    // with no filt type defined, no syntax highlighting needed
    return;
  }

  int prev_sep = 1;

  int i = 0;
  while (i < row->rsize) {
    char c = row->render[i];
    unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

    if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
      if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
          (c == '.' && prev_hl == HL_NUMBER)) {
        row->hl[i] = HL_NUMBER;
        i++;
        prev_sep = 0;
        continue;
      }
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

void editorSelectSyntaxHighlight() {
  E.syntax = NULL;
  if (E.filename == NULL) {
    return;
  }

  char *ext = strrchr(E.filename, '.');

  for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
    struct editorSyntax *s = &HLDB[j];
    unsigned int i = 0;

    while (s->filematch[i]) {
      int is_ext = (s->filematch[i][0] == '.');
      if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
          (!is_ext && strstr(E.filename, s->filematch[i]))) {
        E.syntax = s;

        for (int filerow = 0; filerow < E.numrows; filerow++) {
          editorUpdateSyntax(&E.row[filerow]);
        }

        return;
      }
      i++;
    }
  }
}
