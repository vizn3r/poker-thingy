#include "poker.h"
#include "table.h"
#include "tui.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define NUM_PLAYERS 5
#define STARTING_BAL 1000

void poker_deal(void);

struct poker_game *poker_init(void) {
  struct poker_game *game = (struct poker_game *)malloc(sizeof(struct poker_game));

  game->n_players = NUM_PLAYERS;

  game->players = (struct poker_player **)malloc(sizeof(struct poker_player *) * game->n_players);
  game->board = (struct poker_board *)malloc(sizeof(struct poker_board));

  for (size_t i = 0; i < game->n_players; i++) {
    game->players[i] = (struct poker_player *)malloc(sizeof(struct poker_player));
    game->players[i]->id = i;
    game->players[i]->money = STARTING_BAL;
    game->players[i]->role = PLAYER_NORMAL;
  }

  game->state = POKER_SMALL_BLIND;
  game->board->n_cards = 0;

  return game;
}

void poker_free(struct poker_game *game) {
  free(game->players);
  free(game->board);
  free(game);
}

void poker_render_cards(struct tui_ui *ui, struct poker_game *game) {
  for (size_t i = 0; i < game->board->n_cards; i++) {
    tui_centered_ascii(ui, ui->w / 2 + (i - 2) * 8, (ui->h / 2) - 1, &ascii_base_card);
  }
}

bool poker_play(struct tui_ui *ui, struct poker_game *game) {
  poker_render_cards(ui, game);

  // The poker state machine
  switch (game->state) {
  case POKER_SMALL_BLIND:
    break;
  case POKER_BIG_BLIND:
    break;
  case POKER_PREFLOP:
    break;
  case POKER_FLOP:
    break;
  case POKER_TURN:
    break;
  case POKER_RIVER:
    break;
  case POKER_SHOW_CARDS:
    break;
  case POKER_ROUND_END:
    break;
  }

  return false;
}
