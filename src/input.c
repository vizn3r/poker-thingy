#include "input.h"
#include <termios.h>
#include <unistd.h>

struct termios oldt, newt;
void input_enable_raw_mode(void) {

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void input_disable_raw_mode(void) {
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

int input_get_key(void) {
  char c;
  if (read(STDIN_FILENO, &c, 1) <= 0)
    return 0;

  if (c == 27) {
    char seq[4];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return INPUT_KEY_ESC;
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return INPUT_KEY_ESC;

    if (seq[0] == '[') {
      switch (seq[1]) {
      case 'A':
        return INPUT_KEY_UP;
      case 'B':
        return INPUT_KEY_DOWN;
      case 'C':
        return INPUT_KEY_RIGHT;
      case 'D':
        return INPUT_KEY_LEFT;
      default:
        return 0; // unknown escape sequence, ignore
      }
    }
    return INPUT_KEY_ESC;
  }
  return c;
}
