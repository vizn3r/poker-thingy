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

  game->players = (struct poker_player **)calloc(game->n_players, sizeof(struct poker_player *));
  game->board = (struct poker_board *)malloc(sizeof(struct poker_board));
  game->discard = (struct table_deck *)malloc(sizeof(struct table_deck));
  game->deck = table_deck_init(1);

  table_deck_shuffle(game->deck);

  for (size_t i = 0; i < game->n_players; i++) {
    game->players[i] = (struct poker_player *)malloc(sizeof(struct poker_player));
    game->players[i]->money = STARTING_BAL;
    game->players[i]->role = PLAYER_NORMAL;
  }

  game->main_player = game->players[0];
  if (game->main_player == NULL) {
    return NULL;
  }
  game->dealer = 0;

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

  uint16_t sb = game->dealer;
  uint16_t bb = game->dealer + 1;

  // The poker state machine
  switch (game->state) {
  case POKER_SMALL_BLIND:
    game->players[sb]->role = PLAYER_SMALL_BLIND;
    game->players[bb]->role = PLAYER_BIG_BLIND;

    for (uint16_t i = 0; i < game->n_players; i++) {
      if (game->players[i]->role != PLAYER_SMALL_BLIND || game->players[i]->role != PLAYER_BIG_BLIND) {
        game->players[i]->role = PLAYER_NORMAL;
      }

      game->players[i]->cards[0] = table_deck_draw(game->deck);
      game->players[i]->cards[1] = table_deck_draw(game->deck);
    }
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

  if (game->main_player->cards[0] == NULL) {
    tui_centered_text(ui, ui->w / 2, ui->h / 2, "No cards left");
    return false;
  }

  // Show main player's cards
  struct tui_ascii *main_player_card1 = table_card_ascii(game->main_player->cards[0]);
  struct tui_ascii *main_player_card2 = table_card_ascii(game->main_player->cards[1]);

  tui_centered_ascii(ui, (ui->w / 2) - 5, ui->h / 2, main_player_card1);
  tui_centered_ascii(ui, (ui->w / 2) + 5, ui->h / 2, main_player_card2);

  free(main_player_card1);
  free(main_player_card2);

  return true;
}
