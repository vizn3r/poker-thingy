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
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

int input_get_key(void) {
  char c;
  read(STDIN_FILENO, &c, 1);
  return c;
}
