#include <termios.h>
#include <unistd.h>

void enableRowMode() {
  struct termios raw;

  tcgetattr(STDIN_FILENO, &raw);

  /*
   * ECHO feature causes each key yo type to be printed to the terminal
   * c_lflag field is for "local flags"
   * c_iflag field is for "input flags"
   * c_oflag field is for "output flags"
   * c_cflag field is for "control flags"
   *
   * To disable ECHO, we remove the ECHO flag from the c_lflag field with a
   * bitwise AND operation
   */
  raw.c_lflag &= ~(ECHO);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRowMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    ;
  return 0;
}
