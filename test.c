#include "src/input.h"
#include <stdio.h>

int main(void) {
  input_enable_raw_mode();
  int i = 0;
  while ((i = input_get_key()) != INPUT_KEY_ESC) {
    if (i != -1)
      printf("%d\n", i);
  }
  input_disable_raw_mode();
  return 0;
}
