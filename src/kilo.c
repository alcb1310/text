/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

/***
 * Prints an error message and exits
 *
 * @param s the error message
 */
void die(const char *s) {
  write(STDERR_FILENO, "\x1b[2J", 4); // clear screen
  write(STDERR_FILENO, "\x1b[H", 3);  // cursor home

  perror(s);
  exit(EXIT_FAILURE);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("disableRawMode: tcsetattr");
  }
}

void enableRowMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    die("enableRawMode: tcgetattr");
  }

  /*
   * atexit() is a function that is called when the program exits by either
   * finishing the program or calling the exit() function
   */
  atexit(disableRawMode);

  struct termios raw = orig_termios;

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
char editorReadKey() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) {
      die("editorReadKey: read");
    }
  }

  return c;
}

/*** output ***/

/***
 * Refreshes the screen
 * https://vt100.net/docs/vt100-ug/chapter3.html#ED
 */
void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

/***
 * Waits for a key press and then handles it
 * */
void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // cursor home

    exit(EXIT_SUCCESS);
    break;
  }
}

/*** init ***/

int main() {
  enableRowMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
