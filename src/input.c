#include "input.h"
#include <stdbool.h>
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

int input = -1;
bool consume_next = false;
int input_get_key(void) {
  return input;
}

void input_consume(void) {
  input = -1;
}

void input_consume_next(void) {
  consume_next = true;
}

void input_check(void) {
  if (consume_next) {
    consume_next = false;
    input = -1;
    return;
  }
  char c;
  if (read(STDIN_FILENO, &c, 1) <= 0)
    input = -1;

  if (c == 27) {
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) <= 0)
      input = INPUT_KEY_ESC;
    if (read(STDIN_FILENO, &seq[1], 1) <= 0)
      input = INPUT_KEY_ESC;

    if (seq[0] == '[') {
      switch (seq[1]) {
      case 'A':
        input = INPUT_KEY_UP;
        break;
      case 'B':
        input = INPUT_KEY_DOWN;
        break;
      case 'C':
        input = INPUT_KEY_RIGHT;
        break;
      case 'D':
        input = INPUT_KEY_LEFT;
        break;
      default:
        input = -1;
        break;
      }
    }
    input = INPUT_KEY_ESC;
  }

  input = c;
}

// int input_get_key(void) {
//   if (input >= 0)
//     return input;
//   char c;
//   if (read(STDIN_FILENO, &c, 1) <= 0)
//     return -1;
//
//   if (c == 27) {
//     char seq[2];
//     if (read(STDIN_FILENO, &seq[0], 1) <= 0)
//       return INPUT_KEY_ESC;
//     if (read(STDIN_FILENO, &seq[1], 1) <= 0)
//       return INPUT_KEY_ESC;
//
//     if (seq[0] == '[') {
//       switch (seq[1]) {
//       case 'A':
//         return INPUT_KEY_UP;
//       case 'B':
//         return INPUT_KEY_DOWN;
//       case 'C':
//         return INPUT_KEY_RIGHT;
//       case 'D':
//         return INPUT_KEY_LEFT;
//       default:
//         return -1;
//       }
//     }
//     return INPUT_KEY_ESC;
//   }
//
//   return c;
// }
