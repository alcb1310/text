/*** includes ***/

// this macros will tell the includes what features to expose
#include <stdarg.h>
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <asm-generic/ioctls.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*** defines ***/

#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOPS 8

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

enum editorMode { NORMAL_MODE = 1, INSERT_MODE };

/*** data ***/

typedef struct erow {
  int size;
  int rsize;
  char *chars;
  char *render;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  enum editorMode mode;
  erow *row;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/
void disableRawMode();
void editorSetStatusMessage(const char *fmt, ...);

/***
 * Prints an error message and exits
 *
 * @param s the error message
 */
void die(const char *s) {
  write(STDERR_FILENO, "\x1b[2J", 4); // clear screen
  write(STDERR_FILENO, "\x1b[H", 3);  // cursor home

  perror(s);
  disableRawMode();
  exit(EXIT_FAILURE);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("disableRawMode: tcsetattr");
  }
}

void enableRowMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
    die("enableRawMode: tcgetattr");
  }

  /*
   * atexit() is a function that is called when the program exits by either
   * finishing the program or calling the exit() function
   */
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

  /*
   * c_iflag field is for "input flags"
   *
   * - BRKINT flag is for "send SIGINTR when receiving a break signal"
   * - INPCK flag is for "enable parity checking"
   * - ISTRIP flag causes the 9th bit of each input byt to be stripped, meaning
   *   it will set it to 0
   * - ICRNL flag is for "translate carriage-return to newline" now CTRL-M will
   *   will be read as 13 and ENTER will be read as 10
   * - IXON flag is for "enable start/stop output control" disables CTRL-S and
   *   CTRL-Q default behavior
   */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  /*
   * c_oflag field is for "output flags"
   *
   * - OPOST flag is for "enable output processing" to translate \n to \r\n
   *
   */
  raw.c_oflag &= ~(OPOST);

  /*
   * c_lflag field is for "local flags"
   *
   * - CS8 is not a flag, it is a bit mask to set the number of data bits to 8
   */
  raw.c_cflag |= (CS8);

  /*
   * c_lflag field is for "local flags"
   * c_cflag field is for "control flags"
   *
   * - To disable ECHO, we remove the ECHO flag from the c_lflag field with a
   *   bitwise AND operation
   * - ICANON flag is for "canonical mode" which is used to read one line at a
   *   time by disableling it we will reading byte by byte
   * - ISIG flag is for "send signals for special keys", by default CTRL-C will
   *   send a SIGINT signal to the current process which causes it to terminate
   *   and CTRL-Z will send a SIGTSTP signal to the current process which causes
   *   it to suspend
   * - IEXTEN flag is for "enable extended functions"
   *
   * The ~ operator is a bitwise NOT to disable the flags
   */
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  /*
   * c_cc field is for "special characters"
   *
   * - VMIN is the minimum number of characters to be read before the read()
   *   function returns
   * - VTIME is the maximum number of milliseconds to wait before the read()
   *   function returns
   */
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("enableRawMode: tcsetattr");
  }
}

/***
 * Wait for a key to be pressed and return it
 */
int editorReadKey() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) {
      die("editorReadKey: read");
    }
  }

  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return '\x1b';
    }

    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return '\x1b';
    }

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) {
          return '\x1b';
        }
        if (seq[2] == '~') {
          switch (seq[1]) {
          case '1':
            return HOME_KEY;
          case '3':
            return DEL_KEY;
          case '4':
            return END_KEY;
          case '5':
            return PAGE_UP;
          case '6':
            return PAGE_DOWN;
          case '7':
            return HOME_KEY;
          case '8':
            return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
        case 'A':
          return ARROW_UP;
        case 'B':
          return ARROW_DOWN;
        case 'C':
          return ARROW_RIGHT;
        case 'D':
          return ARROW_LEFT;
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
      case 'H':
        return HOME_KEY;
      case 'F':
        return END_KEY;
      }
    }

    return '\x1b';
  }

  return c;
}

/***
 * Gets the cursor's position
 */
int getCursorPosition(int *rows, int *cols) {
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    return -1;
  }

  char buf[32];
  unsigned int i = 0;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) {
      break;
    }

    if (buf[i] == 'R') {
      break;
    }

    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') {
    return -1;
  }

  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
    return -1;
  }

  return 0;
}

/***
 * Gets the current terminal's window size
 */
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 13) != 13) {
      return -1;
    }
    return getCursorPosition(rows, cols);
  }

  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
}

/*** row operations ***/
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
}

/***
 * Appends a new row to the end of the row array
 */
void editorAppendRow(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

  int at = E.numrows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  editorUpdateRow(&E.row[at]);

  E.numrows++;
}

/*** file i/o ***/

/***
 * Reads a file into a buffer
 *
 * @param filename The name of the file to open
 */
void editorOpen(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    die("editorOpen: fopen");
  }

  E.filename = strdup(filename);

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 &&
           (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }

    editorAppendRow(line, linelen);
  }

  free(line);
  fclose(fp);
}

/*** append buffer ***/

struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}

/***
 * Appends a character to the buffer
 */
void abAppend(struct abuf *ab, char *s, int len) {
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL) {
    // Memory allocation failed
    return;
  }

  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

/***
 * Free the buffer memmory
 */
void abFree(struct abuf *ab) { free(ab->b); }

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

      abAppend(ab, &E.row[filerow].render[E.coloff], len);
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
  int len = snprintf(lstatus, sizeof(lstatus), " %s %.20s - %d lines",
                     E.mode == NORMAL_MODE ? "[NORMAL]" : "[INSERT]",
                     E.filename ? E.filename : "[No Name]", E.numrows);
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
  switch (c) {
  case 'q':
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // cursor home

    exit(EXIT_SUCCESS);
    break;

  case DEL_KEY:
    // TODO: implement
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

  case 'i':
    E.mode = INSERT_MODE;
    editorSetStatusMessage("Press ESC to enter normal mode");
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
    editorSetStatusMessage("HELP: (Ctrl-Q | q) = quit | (i) = Insert Mode");
    break;

  case ARROW_UP:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_LEFT:
    editorMoveCursor(c);
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

  editorSetStatusMessage("HELP: (Ctrl-Q | q) = quit | (i) = Insert Mode");

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
