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
enum game_state state = MENU;

struct poker_game *poker_game = NULL;

void signal_handler(int signal);
void quitter(void);

int main(void) {
  ui = tui_init();

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);
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
  for (;;) {
    tui_clear(ui);
    menu_log("Select input");

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
      poker_play(ui, poker_game);
      poker_display(ui, poker_game);
    }

    tui_render(ui);
    key = input_get_key();
    switch ((state << 8) | key) {
    case (MENU << 8) | 'q':
    case (MENU << 8) | 'Q':
    case (POKER << 8) | 'q':
    case (POKER << 8) | 'Q':
    case (MENU << 8) | INPUT_KEY_ESC:
    case (POKER << 8) | INPUT_KEY_ESC:
      goto exit;
      break;
    case (MENU << 8) | 'p':
    case (MENU << 8) | 'P':
      state = POKER;
      break;
    case (POKER << 8):
      poker_input(poker_game, key);
      break;
    default:
      continue;
    }
  }

exit:
  quitter();

  return 0;
}

void quitter(void) {
  input_disable_raw_mode();
  menu_free();
  tui_clear(ui);
  tui_free(ui);
  exit(EXIT_SUCCESS);
}

void signal_handler(int signal) {
  switch (signal) {
  case SIGINT:
  case SIGQUIT:
  case SIGTERM:
    quitter();
    break;
  case SIGWINCH:
    tui_resize(ui);
    break;
  default:
    break;
  }
}
