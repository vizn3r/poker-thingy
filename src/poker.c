#include "poker.h"
#include "table.h"
#include "tui.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NUM_PLAYERS 5
#define STARTING_BAL 1000
#define SMALL_BLIND 1
#define BIG_BLIND 2

// clang-format off
const struct tui_ascii ascii_chip = {
  .buff = (char *[]){
  "o-o",
  "|1|",
  "o-o"
  },
  .w = 3,
  .h = 3,
};
// clang-format on

void poker_deal(void);

struct poker_game *poker_init(void) {
  struct poker_game *game = (struct poker_game *)malloc(sizeof(struct poker_game));

  game->n_players = NUM_PLAYERS;
  game->small_blind = SMALL_BLIND;
  game->big_blind = BIG_BLIND;

  game->players = (struct poker_player **)calloc(game->n_players, sizeof(struct poker_player *));
  game->discard = (struct table_deck *)malloc(sizeof(struct table_deck));
  game->deck = table_deck_init(1);

  table_deck_shuffle(game->deck);

  for (size_t i = 0; i < game->n_players; i++) {
    game->players[i] = (struct poker_player *)malloc(sizeof(struct poker_player));
    game->players[i]->money = STARTING_BAL;
    game->players[i]->role = PLAYER_NORMAL;
    game->players[i]->action = PLAYER_ACTION_NONE;
    game->players[i]->folded = false;
  }

  game->main_player = game->players[0];
  if (game->main_player == NULL) {
    return NULL;
  }
  game->dealer = 0;

  game->state = POKER_DEAL;
  game->n_cards = 0;

  return game;
}

void poker_free(struct poker_game *game) {
  free(game->players);
  free(game->board);
  free(game);
}

// Handle all the displaying of the poker game
// TODO:
//   - Show board
//   - Show main player's cards
//   - Show dealer/sb/bb chip
//   - Show pot/side pots
//   - Show all player's chips
//   - Show player's available actions
void poker_display(struct tui_ui *ui, struct poker_game *game) {
  struct poker_player *mp = game->main_player;
  // Show board
  for (uint8_t i = 0; i < game->n_cards; i++) {
    struct tui_ascii *card = table_card_ascii(game->cards[i]);
    tui_centered_ascii(ui, ui->w / 2 + (i - 2) * 8, (ui->h / 2) - 1, card);
    free(card);
  }

  // Show main player's cards
  if (mp->cards[0] == NULL) {
    tui_centered_text(ui, ui->w / 2, ui->h / 2, "No cards left");
  }
  struct tui_ascii *mp_card1 = table_card_ascii(mp->cards[0]);
  struct tui_ascii *mp_card2 = table_card_ascii(mp->cards[1]);
  tui_centered_ascii(ui, (ui->w / 2) - 4, ui->h / 2 + ui->h / 4, mp_card1);
  tui_centered_ascii(ui, (ui->w / 2) + 4, ui->h / 2 + ui->h / 4, mp_card2);
  free(mp_card1);
  free(mp_card2);

  // Show dealer/sb/bb chip
  struct tui_ascii *role_chip = (struct tui_ascii *)malloc(sizeof(struct tui_ascii));
  role_chip->w = ascii_chip.w;
  role_chip->h = ascii_chip.h;
  role_chip->buff = (char **)malloc(sizeof(char *) * role_chip->h);
  for (uint16_t i = 0; i < role_chip->h; i++) {
    role_chip->buff[i] = (char *)malloc(sizeof(char) * role_chip->w);
    memcpy(role_chip->buff[i], ascii_chip.buff[i], sizeof(struct tui_ascii));
  }

  switch (mp->role) {
  case PLAYER_BIG_BLIND:
    role_chip->buff[1][1] = 'B';
    tui_centered_ascii(ui, ui->w / 2, ui->h / 2 + ui->h / 4, role_chip);
    break;
  case PLAYER_SMALL_BLIND:
    role_chip->buff[1][1] = 'S';
    tui_centered_ascii(ui, ui->w / 2, ui->h / 2 + ui->h / 4, role_chip);
    break;
  case PLAYER_DEALER:
    role_chip->buff[1][1] = 'D';
    tui_centered_ascii(ui, ui->w / 2, ui->h / 2 + ui->h / 4, role_chip);
    break;
  default:
    break;
  }

  free(role_chip);
}

// Check what the player can do
void poker_check_player_actions(struct poker_game *game, size_t player_id) {
  switch (game->players[player_id]->role) {
  }
}

bool poker_play(struct tui_ui *ui, struct poker_game *game) {
  size_t sb = game->dealer;
  size_t bb = (game->dealer + 1) % game->n_players;
  game->last_aggressor = (bb + 1) % game->n_players; // BB is by default the first last aggressor

  // The poker state machine
  switch (game->state) {

  // Players are dealt
  case POKER_DEAL:
    game->players[sb]->role = PLAYER_SMALL_BLIND;
    game->players[bb]->role = PLAYER_BIG_BLIND;

    for (uint16_t i = 0; i < game->n_players; i++) {
      if (game->players[i]->role != PLAYER_SMALL_BLIND || game->players[i]->role != PLAYER_BIG_BLIND) {
        game->players[i]->role = PLAYER_NORMAL;
      }

      game->players[i]->cards[0] = table_deck_draw(game->deck);
      game->players[i]->cards[1] = table_deck_draw(game->deck);
    }

    game->state = POKER_SMALL_BLIND;
    // break;

  // Small blind player pays
  case POKER_SMALL_BLIND:
    game->pot += game->small_blind;
    game->players[sb]->money -= game->small_blind;

    game->state = POKER_BIG_BLIND;
    // break;

  // Big blind player pays
  case POKER_BIG_BLIND:
    game->pot += game->big_blind;
    game->players[bb]->money -= game->big_blind;

    game->state = POKER_PREFLOP;
    // break;

  // Players call/check/fold/raise
  case POKER_PREFLOP:
    break;

  // First 3 cards are dealt on the board, players check/fold/raise/call
  case POKER_FLOP:
    for (uint8_t i = 0; i < 3; i++) {
      game->cards[i] = table_deck_draw(game->deck);
    }
    break;

  // 4th card is dealt on the board, players check/fold/raise/call
  case POKER_TURN:
    break;

  // 5th card is dealt on the board, players check/fold/raise/call
  case POKER_RIVER:
    break;

  // Evaulation of the board, players get payed out
  case POKER_SHOW_CARDS:
    break;

  // End of the round, reset deck, dealer, and players
  case POKER_ROUND_END:
    game->dealer = (game->dealer + 1) % game->n_players;

    for (uint16_t i = 0; i < game->n_players; i++) {
      game->players[i]->cards[0] = NULL;
      game->players[i]->cards[1] = NULL;
    }

    game->state = POKER_DEAL;
    break;
  }
  return true;
}
