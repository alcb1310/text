#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
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
#define KILO_TAB_STOPS 2
#define KILO_QUIT_TIMES 2
#define DEFAULT_MESSAGE                                                        \
  "HELP: (Ctrl-Q | q) = quit | (Ctrl-S | w) = save | (i) = Insert Mode | "     \
  "(Ctrl-F | /) = find"

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
  BACKSPACE = 127,
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

enum editorMode { NORMAL_MODE, INSERT_MODE };

enum editorHighlight { HL_NORMAL = 0, HL_NUMBER, HL_MATCH };

/*** data ***/

typedef struct erow {
  int size;
  int rsize;
  char *chars;
  char *render;
  unsigned char *hl;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  int dirty;
  enum editorMode mode;
  erow *row;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct termios orig_termios;
};

extern struct editorConfig E;

#endif // !#ifndef TYPEDEFS_H_
