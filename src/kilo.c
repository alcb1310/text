#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enableRowMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);

  /*
   * atexit() is a function that is called when the program exits by either
   * finishing the program or calling the exit() function
   */
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  /*
   * ECHO feature causes each key yo type to be printed to the terminal
   * c_lflag field is for "local flags"
   * c_iflag field is for "input flags"
   * c_oflag field is for "output flags"
   * c_cflag field is for "control flags"
   *
   * - To disable ECHO, we remove the ECHO flag from the c_lflag field with a
   *   bitwise AND operation
   * - ICANON flag is for "canonical mode" which is used to read one line at a
   *   time by disableling it we will reading byte by byte
   */
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRowMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    if (iscntrl(c)) {
      // This is a control character (0 to 31 or 127)
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
  }

  return 0;
}
