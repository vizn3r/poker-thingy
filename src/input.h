#pragma once

#define INPUT_KEY_ESC 27
#define INPUT_KEY_ENTER 13
#define INPUT_KEY_SPACE 32
#define INPUT_KEY_UP 1000   // arrow keys don't have real ASCII values
#define INPUT_KEY_DOWN 1001 // use out-of-range ints instead
#define INPUT_KEY_LEFT 1002
#define INPUT_KEY_RIGHT 1003
#define INPUT_KEY_TAB 9
#define INPUT_KEY_BACKSPACE 8
#define INPUT_KEY_DELETE 127
void input_enable_raw_mode(void);

void input_disable_raw_mode(void);

int input_get_key(void);

void input_signal_handler(int signal);
