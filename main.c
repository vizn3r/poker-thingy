#include "input.h"
#include "poker.h"
#include "table.h"
#include "tui.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define GAME_TITLE "A Casino Game"

enum game_state {
  MENU,
  POKER
};

struct tui_ui *ui;
int key;
uint16_t wc, hc;
enum game_state state = MENU;

struct poker_game *poker_game = NULL;

void signal_handler(int signal);
void print_menu(void);

int main(void) {
  ui = tui_init();

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);

  input_enable_raw_mode();

  // struct table_deck *deck = table_deck_init(1);

  wc = ui->w / 2;
  hc = ui->h / 2;
  char ws[10], hs[10];
  snprintf(ws, sizeof(ws), "w: %d", ui->w);
  snprintf(hs, sizeof(hs), "h: %d", ui->h);

  for (;;) {
    tui_clear(ui);
    // tui_centered_text(ui, wc, hc, "+");

    if (state == MENU)
      print_menu();

    if (state == POKER) {
      if (poker_game == NULL) {
        poker_game = poker_init();
      }
      poker_play(ui, poker_game);
    }

    tui_text(ui, 1, ui->h - 3, ws);
    tui_text(ui, 1, ui->h - 2, hs);
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
      break;
    }
  }

exit:

  tui_clear(ui);
  tui_free(ui);
  input_disable_raw_mode();

  return 0;
}

void print_menu(void) {
  tui_centered_text(ui, wc, hc - 1, GAME_TITLE);
  tui_centered_text(ui, wc, hc + 1, "Press [P] to start");
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
