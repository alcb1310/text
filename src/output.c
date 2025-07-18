#include "output.h"
#include "append.h"
#include "row.h"
#include "syntax.h"
#include "typedefs.h"
#include <stdio.h>

/***
 * Scrolls vertically the screen
 */
void editorScroll() {
  E.rx = E.cx;
  if (E.cy < E.numrows) {
    E.rx = editorRowToRx(&E.row[E.cy], E.cx);
  }

  // vertical scroll
  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }

  // horizontal scroll
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

/*
 * Draws information to the screen
 */
void editorDrawRows(struct abuf *ab) {
  for (int y = 0; y < E.screenrows; y++) {
    int filerow = y + E.rowoff;

    editorDrawSignColumn(ab, filerow);

    if (filerow >= E.numrows) {
      if (E.numrows == 0 && filerow == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
                                  "Kilo Editor -- Version %s", KILO_VERSION);

        if (welcomelen > E.screencols) {
          welcomelen = E.screencols;
        }

        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          // Add some padding
          abAppend(ab, "~", 1);
          padding--;
        }

        while (padding--) {
          abAppend(ab, " ", 1);
        }

        abAppend(ab, welcome, welcomelen);
      } else {
        abAppend(ab, "~", 1);
      }
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0) {
        len = 0;
      }
      if (len > E.screencols) {
        len = E.screencols;
      }
      char *c = &E.row[filerow].render[E.coloff];
      unsigned char *hl = &E.row[filerow].hl[E.coloff];
      int current_color = -1;

      for (int j = 0; j < len; j++) {
        if (iscntrl(c[j])) {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abAppend(ab, "\x1b[7m", 4);
          abAppend(ab, &sym, 1);
          abAppend(ab, "\x1b[m", 3);
          if (current_color != -1) {
            char buf[16];
            int clen = snprintf(buf, sizeof buf, "\x1b[%dm", current_color);
            abAppend(ab, buf, clen);
          }
        } else if (hl[j] == HL_NORMAL) {
          if (current_color != -1) {
            abAppend(ab, "\x1b[39m", 5);
            current_color = -1;
          }
          abAppend(ab, &c[j], 1);
        } else {
          int color = editorSyntaxToColor(hl[j]);
          if (color != current_color) {
            current_color = color;
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
            abAppend(ab, buf, clen);
          }
          abAppend(ab, &c[j], 1);
        }
      }
      abAppend(ab, "\x1b[39m", 5);
    }

    abAppend(ab, "\x1b[K", 3); // clear line
    abAppend(ab, "\r\n", 2);
  }
}

/***
 * Draws the status bar
 * https://vt100.net/docs/vt100-ug/chapter3.html#ED
 *
 * @param *ab The append buffer
 */
void editorDrawStatusBar(struct abuf *ab) {
  abAppend(ab, "\x1b[7m", 4); // reverse video bg color = white && text black

  char lstatus[80], rstatus[80];
  int len = snprintf(lstatus, sizeof(lstatus), " %s %.20s - %d lines %s",
                     E.mode == NORMAL_MODE ? "[NORMAL]" : "[INSERT]",
                     E.filename ? E.filename : "[No Name]", E.numrows,
                     E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | line %d/%d cols %d/%d",
                      E.syntax ? E.syntax->filetype : "no ft", E.cy + 1,
                      E.numrows, E.rx + 1, E.row ? E.row[E.cy].size + 1 : 1);
  if (len > E.screencols) {
    len = E.screencols;
  }
  abAppend(ab, lstatus, len);
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    }

    abAppend(ab, " ", 1);
    len++;
  }
  abAppend(ab, "\x1b[m", 3); // normal video bg color = black && text white
  abAppend(ab, "\r\n", 2);
}

/***
 * Draws the message bar
 *
 * @param *ab The append buffer
 */
void editorDrawMessageBar(struct abuf *ab) {
  abAppend(ab, "\x1b[K", 3); // clear line
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) {
    msglen = E.screencols;
  }

  if (msglen && time(NULL) - E.statusmsg_time < 5) {
    abAppend(ab, E.statusmsg, msglen);
  }
}

/***
 * Refreshes the screen
 * https://vt100.net/docs/vt100-ug/chapter3.html#ED
 */
void editorRefreshScreen() {
  editorScroll();

  struct abuf ab = ABUF_INIT;
  abAppend(&ab, "\x1b[?25l", 6); // hide cursor
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
           (E.rx - E.coloff) + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6); // show cursor

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

/***
 * Draws the sign column with the row number
 *
 * @param numrow current row number
 */
void editorDrawSignColumn(struct abuf *ab, int numrow) {
  if (KILO_SIGN_COLUMN == 0) {
    return;
  }

  if (numrow < E.numrows) {
    char buf[32];
    int rowlength = snprintf(buf, sizeof(buf), "%d ", numrow + 1);
    if (rowlength > E.screencols) {
      rowlength = E.screencols;
    }
    int len = KILO_SIGN_COLUMN - rowlength - 1;
    if (len < 0) {
      len = 0;
    }
    for (int i = 0; i < len; i++) {
      abAppend(ab, " ", 1);
    }
    abAppend(ab, buf, rowlength);
    abAppend(ab, " ", 1);
  } else {
    for (int i = 0; i < KILO_SIGN_COLUMN; i++) {
      abAppend(ab, " ", 1);
    }
  }
}
