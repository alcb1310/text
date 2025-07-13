/*** includes ***/

// this macros will tell the includes what features to expose
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "append.h"
#include "editor.h"
#include "file.h"
#include "find.h"
#include "row.h"
#include "syntax.h"
#include "terminal.h"
#include "typedefs.h"

struct editorConfig E;

/*** prototypes ***/

void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen();
char *editorPrompt(char *prompt, void (*callback)(char *, int));

/*** output ***/

/***
 * Scrolls vertically the screen
 */
void editorScroll() {
  E.rx = 0;
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
        if (hl[j] == HL_NORMAL) {
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
  int rlen =
      snprintf(rstatus, sizeof(rstatus), "line %d/%d cols %d/%d", E.cy + 1,
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

/*** input ***/

/***
 * Show a prompt for user to interact with
 *
 * @param *prompt the question to ask the user
 * @return a string with the user's response
 */
char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();

    int c = editorReadKey();
    if (c == DEL_KEY || c == BACKSPACE) {
      if (buflen != 0) {
        buf[--buflen] = '\0';
      }
    } else if (c == '\x1b') {
      editorSetStatusMessage("");
      if (callback) {
        callback(buf, c);
      }
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
        editorSetStatusMessage("");
        if (callback) {
          callback(buf, c);
        }
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }

    if (callback) {
      callback(buf, c);
    }
  }
}

/***
 * Moves the cursor
 */
void editorMoveCursor(int key) {
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  switch (key) {
  case ARROW_LEFT:
    if (E.cx != 0) {
      E.cx--;
    } else if (E.cy > 0) {
      E.cy--;
      E.cx = E.row[E.cy].size;
    }
    break;
  case ARROW_RIGHT:
    if (row != NULL && E.cx < row->size) {
      E.cx++;
    } else if (row != NULL && E.cx == row->size) {
      E.cy++;
      E.cx = 0;
    }
    break;
  case ARROW_UP:
    if (E.cy != 0) {
      E.cy--;
    }
    break;
  case ARROW_DOWN:
    if (E.cy < E.numrows) {
      E.cy++;
    }
    break;
  }

  // row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  if (E.cy >= E.numrows) {
    row = NULL;
  } else {
    row = &E.row[E.cy];
  }

  int rowlen;
  if (row == NULL) {
    rowlen = 0;
  } else {
    rowlen = row->size;
  }

  if (E.cx > rowlen) {
    E.cx = rowlen;
  }
}

/***
 * Waits for a key press and then handles it in normal mode
 *
 * @param c the key pressed
 * */
void editorNormalProcessKeypress(int c) {
  static int quit_times = KILO_QUIT_TIMES;

  switch (c) {
  case 'q':
  case CTRL_KEY('q'):
    if (E.dirty && quit_times > 0) {
      editorSetStatusMessage("WARNING!!! file has unsaved changes. Press 'q' "
                             "%d more times to quit",
                             quit_times);
      quit_times--;
      return;
    }

    write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // cursor home

    exit(EXIT_SUCCESS);
    break;

  case '\r':
    editorMoveCursor(ARROW_DOWN);
    break;

  case '/':
  case CTRL_KEY('f'):
    editorFind();
    break;

  case CTRL_KEY('s'):
  case 'w':
    editorSave();
    break;

  case '0':
  case HOME_KEY:
    E.cx = 0;
    break;

  case '$':
  case END_KEY:
    if (E.cy < E.numrows) {
      E.cx = E.row[E.cy].size;
    }
    break;

  case CTRL_KEY('u'):
  case PAGE_UP: {
    int times = E.screenrows / 2;
    while (times--) {
      editorMoveCursor(ARROW_UP);
    }
  } break;

  case CTRL_KEY('d'):
  case PAGE_DOWN: {
    int times = E.screenrows / 2;
    while (times--) {
      editorMoveCursor(ARROW_DOWN);
    }
  } break;

  case ARROW_UP:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_LEFT:
    editorMoveCursor(c);
    break;

  case 'k':
    editorMoveCursor(ARROW_UP);
    break;
  case 'j':
    editorMoveCursor(ARROW_DOWN);
    break;
  case 'h':
    editorMoveCursor(ARROW_LEFT);
    break;
  case 'l':
    editorMoveCursor(ARROW_RIGHT);
    break;

  case 'A':
    if (E.cy < E.numrows) {
      E.cx = E.row[E.cy].size;
    }
    E.mode = INSERT_MODE;
    editorSetStatusMessage("Press ESC to enter normal mode");
    break;

  case 'I':
    E.cx = 0;
    // TODO: move to the first non-space character
    E.mode = INSERT_MODE;
    editorSetStatusMessage("Press ESC to enter normal mode");
    break;

  case 'a':
    editorMoveCursor(ARROW_RIGHT);
    // fall through
  case 'i':
    E.mode = INSERT_MODE;
    editorSetStatusMessage("Press ESC to enter normal mode");
    break;

  case 'x':
    editorMoveCursor(ARROW_RIGHT);
    editorDelChar();
    break;
  }
}

/***
 * Waits for a key press and then handles it in insert mode
 *
 * @param c the key pressed
 */
void editorInsertProcessKeypress(int c) {
  switch (c) {
  case '\x1b': // escape
    E.mode = NORMAL_MODE;
    editorSetStatusMessage(DEFAULT_MESSAGE);
    break;

  case '\r':
    editorInsertNewLine();
    break;

  case CTRL_KEY('l'):
    break;

  case BACKSPACE:
  case DEL_KEY:
    if (c == DEL_KEY) {
      editorMoveCursor(ARROW_RIGHT);
    }
    editorDelChar();
    break;

  case ARROW_UP:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_LEFT:
    editorMoveCursor(c);
    break;

  default:
    editorInsertChar(c);
    break;
  }
}

/***
 * Waits for a key press and then handles it
 */
void editorProcessKeypress() {
  int c = editorReadKey();

  switch (E.mode) {
  case NORMAL_MODE:
    editorNormalProcessKeypress(c);
    break;

  case INSERT_MODE:
    editorInsertProcessKeypress(c);
    break;
  }
}

void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

/*** init ***/

/***
 * Initializes the program
 */
void initEditor() {
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.dirty = 0;
  E.mode = NORMAL_MODE;
  E.row = NULL;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
    die("getWindowSize");
  }

  E.screenrows -= 2;
}

int main(int argc, char *argv[]) {
  enableRowMode();
  initEditor();
  if (argc >= 2) {
    editorOpen(argv[1]);
  }

  editorSetStatusMessage(DEFAULT_MESSAGE);

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
