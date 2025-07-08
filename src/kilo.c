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
   * c_iflag field is for "input flags"
   *
   * - ICRNL flag is for "translate carriage-return to newline" now CTRL-M will
   *   will be read as 13 and ENTER will be read as 10
   * - IXON flag is for "enable start/stop output control" disables CTRL-S and
   *   CTRL-Q default behavior
   */
  raw.c_iflag &= ~(ICRNL | IXON);

  /*
   * c_oflag field is for "output flags"
   *
   * - OPOST flag is for "enable output processing" to translate \n to \r\n
   *
   */
  raw.c_oflag &= ~(OPOST);

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

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRowMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    if (iscntrl(c)) {
      // This is a control character (0 to 31 or 127)
      // According to ASCII table
      // http://asciitable.com/
      printf("%d\r\n", c);
    } else {
      // This is a printable character
      printf("%d ('%c')\r\n", c, c);
    }
  }

  return 0;
}
