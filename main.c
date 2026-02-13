#include "input.h"

#define GAME_TITLE "A Casino Game"
#include "menu.h"
#include "poker.h"
#include "table.h"
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

int main(void) {
  ui = tui_init();

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);

  input_enable_raw_mode();

  menu_log("Starting");
  for (;;) {
    tui_clear(ui);
    // tui_centered_text(ui, wc, hc, "+");

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
    }

    tui_render(ui);
    switch (input_get_key()) {
    case 'q':
    case 'Q':
    case INPUT_KEY_ESC:
      goto exit;
      break;
    case 'p':
    case 'P':
      state = POKER;
    default:
      continue;
    }
  }

exit:

  tui_clear(ui);
  tui_free(ui);
  input_disable_raw_mode();

  return 0;
}

void signal_handler(int signal) {
  switch (signal) {
  case SIGINT:
  case SIGQUIT:
  case SIGTERM:
    input_disable_raw_mode();
    tui_free(ui);
    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}
