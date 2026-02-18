#include "input.h"
#include "menu.h"
#include "poker.h"
#include "tui.h"
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

enum game_state {
  MENU,
  POKER
};

struct tui_ui *ui;
int key;
bool quit_latch = 0;
enum game_state state = MENU;

struct poker_game *poker_game = NULL;

void signal_handler(int signal);
void quitter(void);

int main(void) {
  ui = tui_init();

  // Signals
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGFPE, signal_handler);

  atexit(quitter);

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0; // no SA_RESTART
  sigemptyset(&sa.sa_mask);
  sigaction(SIGWINCH, &sa, NULL);

  input_enable_raw_mode();
  tui_clear(ui);
  tui_resize(ui);
  tui_render(ui);

  menu_log("Starting");
  input_consume_next();
  for (;;) {
    tui_clear(ui);
    menu_log("Select input");
    input_check();

    switch ((state << 8) | input_get_key()) {
    case (MENU << 8) | 'q':
    case (MENU << 8) | 'Q':
    case (POKER << 8) | 'q':
    case (POKER << 8) | 'Q':
    case (MENU << 8) | INPUT_KEY_ESC:
    case (POKER << 8) | INPUT_KEY_ESC:
      input_consume();
      quitter();
      break;
    case (MENU << 8) | 'p':
    case (MENU << 8) | 'P':
      input_consume();
      state = POKER;
      break;
      break;
    default:
      quit_latch = 0;
      break;
    }

    if (state == MENU)
      menu_main_print(ui);

    if (state == POKER) {
      if (poker_game == NULL) {
        poker_game = poker_init();
        if (poker_game == NULL) {
          tui_text(ui, 1, ui->h - 3, "Error: Could not initialize poker game");
          tui_render(ui);
          continue;
        }
      }
      poker_play(ui, poker_game); // Handles input the first time
      poker_play(ui, poker_game); // Handles state the second time
      poker_display(ui, poker_game);
    }

    tui_render(ui);
  }

  return 0;
}

bool quitting = false;
void quitter(void) {
  // Require two presses of quit key
  // To quit instantly, set quit_latch to 1 before calling
  if (!quit_latch) {
    quit_latch = 1;
    return;
  }

  // Prevent multiple quits
  if (quitting)
    return;
  quitting = true;

  input_disable_raw_mode();
  menu_free();
  tui_free(ui);
  menu_print_exit_msg();
  exit(EXIT_SUCCESS);
}

void signal_handler(int signal) {
  switch (signal) {
  case SIGINT:
  case SIGQUIT:
  case SIGTERM:
  case SIGSEGV:
  case SIGABRT:
  case SIGFPE:
    quit_latch = 1;
    quitter();
    break;
  case SIGWINCH:
    tui_resize(ui);
    break;
  default:
    break;
  }
}
