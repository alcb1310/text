#ifndef DEFINES_H
#define DEFINES_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct editorConfig {
  int cx, cy;
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct abuf {
  char *b;
  int len;
};

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)
#define KILO_VERSION "0.0.1"
#define ABUF_INIT {NULL, 0}

/*** data ***/

extern struct editorConfig E;

#endif
