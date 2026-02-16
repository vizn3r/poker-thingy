#include "input.h"
#include <termios.h>
#include <unistd.h>

struct termios oldt, newt;
void input_enable_raw_mode(void) {

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VTIME] = 0;
  newt.c_cc[VMIN] = 1;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void input_disable_raw_mode(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int input_get_key(void) {
  char c;
  if (read(STDIN_FILENO, &c, 1) <= 0)
    return -1;

  if (c == 27) {
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) <= 0)
      return INPUT_KEY_ESC;
    if (read(STDIN_FILENO, &seq[1], 1) <= 0)
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
        return -1;
      }
    }
    return INPUT_KEY_ESC;
  }

  return c;
}
