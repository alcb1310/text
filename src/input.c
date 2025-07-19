#include "input.h"
#include "commands.h"
#include "editor.h"
#include "file.h"
#include "find.h"
#include "output.h"
#include "terminal.h"
#include "typedefs.h"

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
    if (E.cx != KILO_SIGN_COLUMN) {
      E.cx--;
    } else if (E.cy > 0) {
      E.cy--;
      E.cx = E.row[E.cy].size + KILO_SIGN_COLUMN;
    }
    break;
  case ARROW_RIGHT: {
    int size = ((row) ? row->size : 0) + KILO_SIGN_COLUMN;
    if (row != NULL && E.cx < size) {
      E.cx++;
    } else if (row != NULL && E.cx == size) {
      E.cy++;
      E.cx = KILO_SIGN_COLUMN;
    }
  } break;
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
    rowlen = KILO_SIGN_COLUMN;
  } else {
    rowlen = row->size + KILO_SIGN_COLUMN;
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
  switch (c) {
  case ':':
    E.mode = COMMAND_MODE;
    editorCommandMode();
    break;

  case '\r':
    editorMoveCursor(ARROW_DOWN);
    break;

  case '/':
  case CTRL_KEY('f'):
    editorFind();
    break;

  case '0':
  case HOME_KEY:
    E.cx = KILO_SIGN_COLUMN;
    break;

  case '$':
  case END_KEY:
    if (E.cy < E.numrows) {
      E.cx = E.row[E.cy].size + KILO_SIGN_COLUMN;
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
      E.cx = E.row[E.cy].size + KILO_SIGN_COLUMN;
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

  default:
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

void editorCommandMode() {
  char *q = editorPrompt(":%s", NULL);
  if (q != NULL) {
    if (strcmp(q, "q") == 0) {
      quit();
    } else if (strcmp(q, "q!") == 0) {
      force_quit();
    } else if (strcmp(q, "w") == 0) {
      editorSave();
    } else if (strcmp(q, "wq") == 0 || strcmp(q, "x") == 0) {
      editorSave();
      quit();
    }
    free(q);
  }

  E.mode = NORMAL_MODE;
}
