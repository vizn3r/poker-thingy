#include "poker.h"
#include "table.h"
#include "tui.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_PLAYERS 5
#define STARTING_BAL 1000
#define SMALL_BLIND 1
#define BIG_BLIND 2

// clang-format off
const struct tui_ascii ascii_chip = {
  .buff = (char *[]){
    "╭───╮",
    "│ 2 │",
    "╰───╯",
  },
  .w = 5,
  .h = 3,
}; // clang-format on

const char *poker_states[] = {
    "Deal",
    "Small Blind",
    "Big Blind",
    "Preflop",
    "Flop",
    "Turn",
    "River",
    "Showdown",
    "Round End",
};

const char *poker_actions[] = {
    "[F]old",
    "[C]heck/Call",
    "[B]et",
    "[R]aise",
    "[A]ll In",
};

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
    game->players[i]->show_cards = true; // TODO: For now, show cards by default for testing purposes
    game->players[i]->name = NULL;
  }

  game->main_player = game->players[0];
  if (game->main_player == NULL) {
    return NULL;
  }
  game->dealer = 0;

  game->state = POKER_DEAL;
  game->n_cards = 0;
  game->rounds_played = 0;

  return game;
}

void poker_free(struct poker_game *game) {
  for (size_t i = 0; i < game->n_players; i++) {
    free(game->players[i]);
  }
  free(game->players);
  for (size_t i = 0; i < game->n_cards; i++) {
    free(game->cards[i]);
  }
  free(game->deck);
  free(game->discard);
  free(game->main_player);
  free(game->side_pots);
  free(game->board);
  free(game);
}

// Handle all the displaying of the poker game
// TODO:
//   - Show pot/side pots
//   - Show all player's chips
//   - Show player's available actions
//   - Show other player's cards
void poker_display(struct tui_ui *ui, struct poker_game *game) {
  if (game == NULL)
    return;
  struct poker_player *mp = game->main_player;

  // Show board
  for (uint8_t i = 0; i < game->n_cards; i++) {
    struct tui_ascii *card = table_card_ascii(game->cards[i]);
    tui_centered_ascii(ui, ui->w / 2 + (i - 2) * 8, (ui->h / 2) - 1, card);
    tui_ascii_free(card);
  }

  // Show main player's cards
  if (mp->cards[0] != NULL && mp->cards[1] != NULL) {
    struct tui_ascii *mp_card1 = table_card_ascii(mp->cards[0]);
    struct tui_ascii *mp_card2 = table_card_ascii(mp->cards[1]);
    tui_centered_ascii(ui, (ui->w / 2) - 4, ui->h / 2 + ui->h / 4, mp_card1);
    tui_centered_ascii(ui, (ui->w / 2) + 4, ui->h / 2 + ui->h / 4, mp_card2);
    tui_ascii_free(mp_card1);
    tui_ascii_free(mp_card2);
  }

  // Show dealer/sb/bb chip
  struct tui_ascii *role_chip = (struct tui_ascii *)malloc(sizeof(struct tui_ascii));
  role_chip->w = ascii_chip.w;
  role_chip->h = ascii_chip.h;
  role_chip->buff = (char **)malloc(sizeof(char *) * role_chip->h);
  for (uint16_t i = 0; i < role_chip->h; i++) {
    size_t len = strlen(ascii_chip.buff[i]) + 1;
    role_chip->buff[i] = (char *)malloc(len);
    memcpy(role_chip->buff[i], ascii_chip.buff[i], len);
  }

  uint16_t chip_x = tui_gx(ui, 3);
  uint16_t chip_y = tui_gy(ui, 6);
  switch (mp->role) {
  case PLAYER_BIG_BLIND:
    role_chip->buff[1][4] = 'B';
    tui_centered_ascii(ui, chip_x, chip_y, role_chip);
    break;
  case PLAYER_SMALL_BLIND:
    role_chip->buff[1][4] = 'S';
    tui_centered_ascii(ui, chip_x, chip_y, role_chip);
    break;
  case PLAYER_DEALER:
    role_chip->buff[1][4] = 'D';
    tui_centered_ascii(ui, chip_x, chip_y, role_chip);
    break;
  default:
    break;
  }
  tui_ascii_free(role_chip);

  // Show current poker state
  tui_centered_text(ui, ui->w / 2, 8 / 9 * ui->h, (char *)poker_states[game->state]);
  char rounds_played[32] = "";
  sprintf(rounds_played, "Round %d", game->rounds_played);
  tui_centered_text(ui, ui->w / 2, 8 / 9 * ui->h + 1, rounds_played);

  // Show main player's actions
  char actions[64] = "";
  for (enum poker_player_action action = PLAYER_ACTION_FOLD; action < PLAYER_ACTION_MAX; action++) {
    if (mp->possible_actions & action) {
      strcat(actions, (char *)poker_actions[action - 1]);
    }
  }
  tui_centered_text(ui, ui->w / 2, 8 / 9 * ui->h + 2, actions);

  // Show other player's cards
}

int last_key = -1;
void poker_input(struct poker_game *game, int key) {
  (void)game;
  last_key = key;
}

// Check what the player can do
// Player can:
//  PLAYER_ACTION_NONE       - default state or when player folded or went all in
//  PLAYER_ACTION_FOLD       withdrawing - player fold - cards get returned, bet goes to the pot
//  PLAYER_ACTION_CHECK_CALL coninuing - player checks - passes turn, or calls the current bet, then passes turn
//  PLAYER_ACTION_BET        bet - player calls the current bet, then passes turn
//  PLAYER_ACTION_RAISE      raise - player calls the current bet, then passes turn
//  PLAYER_ACTION_ALL_IN     all in - player goes all in - special rules
//
void poker_check_player_possible_actions(struct poker_game *game, size_t player_id) {
  struct poker_player *player = game->players[player_id];

  // Check if player is folded
  if (player->folded) {
    player->possible_actions = 0;
    return;
  }

  // Check bets
  if (game->current_bet > player->bet) {
    player->possible_actions = PLAYER_ACTION_RAISE | PLAYER_ACTION_ALL_IN | PLAYER_ACTION_CHECK_CALL | PLAYER_ACTION_FOLD;
    return;
  }
  if (game->current_bet == 0) {
    player->possible_actions = PLAYER_ACTION_BET | PLAYER_ACTION_ALL_IN | PLAYER_ACTION_CHECK_CALL | PLAYER_ACTION_FOLD;
    return;
  }

  // Default
  player->possible_actions = PLAYER_ACTION_CHECK_CALL | PLAYER_ACTION_ALL_IN | PLAYER_ACTION_BET | PLAYER_ACTION_FOLD;
}

void poker_player_do_action(struct poker_game *game, size_t player_id, enum poker_player_action action) {
  struct poker_player *player = game->players[player_id];
  switch (action) {
  case PLAYER_ACTION_FOLD:
    player->folded = true;
    break;
  case PLAYER_ACTION_CHECK_CALL:
    // Nothing spectacular
    break;
  case PLAYER_ACTION_BET:
    break;
  case PLAYER_ACTION_RAISE:
    break;
  case PLAYER_ACTION_ALL_IN:
    break;
  default:
    break;
  }
}

bool poker_play(struct tui_ui *ui, struct poker_game *game) {
  (void)ui;
  size_t sb = (game->dealer + 1) % game->n_players;
  size_t bb = (game->dealer + 2) % game->n_players;

  // The poker state machine
  switch (game->state) {

  // Players are dealt
  case POKER_DEAL:
    game->players[sb]->role = PLAYER_SMALL_BLIND;
    game->players[bb]->role = PLAYER_BIG_BLIND;

    for (uint16_t i = 0; i < game->n_players; i++) {
      if (game->players[i]->role != PLAYER_SMALL_BLIND && game->players[i]->role != PLAYER_BIG_BLIND) {
        game->players[i]->role = PLAYER_NORMAL;
      }

      game->players[i]->cards[0] = table_deck_draw(game->deck);
      game->players[i]->cards[1] = table_deck_draw(game->deck);
      if (game->players[i]->cards[0] == NULL || game->players[i]->cards[1] == NULL) {
        tui_text(ui, 1, ui->h - 3, "Error: Could not initialize poker game");
        return false;
      }
    }

    game->state = POKER_SMALL_BLIND;
    game->last_aggressor = (bb + 1) % game->n_players; // BB is by default the first last aggressor

    break;

  // Small blind player pays
  case POKER_SMALL_BLIND:
    game->pot += game->small_blind;
    game->players[sb]->money -= game->small_blind;

    game->state = POKER_BIG_BLIND;
    break;

  // Big blind player pays
  case POKER_BIG_BLIND:
    game->pot += game->big_blind;
    game->players[bb]->money -= game->big_blind;

    game->state = POKER_PREFLOP;
    break;

  // Players call/check/fold/raise
  case POKER_PREFLOP:
    // TODO: For now, just check main player - always idx 0
    poker_check_player_possible_actions(game, 0);
    game->state = POKER_FLOP;
    break;

  // First 3 cards are dealt on the board, players check/fold/raise/call
  case POKER_FLOP:
    for (uint8_t i = 0; i < 3; i++) {
      game->cards[i] = table_deck_draw(game->deck);
    }
    game->n_cards = 3;

    // TODO: For now, just check main player - always idx 0
    poker_check_player_possible_actions(game, 0);

    game->state = POKER_TURN;
    break;

  // 4th card is dealt on the board, players check/fold/raise/call
  case POKER_TURN:
    game->cards[3] = table_deck_draw(game->deck);
    game->n_cards = 4;

    // TODO: For now, just check main player - always idx 0
    poker_check_player_possible_actions(game, 0);

    game->state = POKER_RIVER;
    break;

  // 5th card is dealt on the board, players check/fold/raise/call
  case POKER_RIVER:
    game->cards[4] = table_deck_draw(game->deck);
    game->n_cards = 5;

    // TODO: For now, just check main player - always idx 0
    poker_check_player_possible_actions(game, 0);

    game->state = POKER_SHOW_CARDS;
    break;

  // Evaulation of the board, players get payed out
  case POKER_SHOW_CARDS:
    game->state = POKER_ROUND_END;
    break;

  // End of the round, reset deck, dealer, and players
  case POKER_ROUND_END:
    // Advance to next dealer
    game->dealer = (game->dealer + 1) % game->n_players;

    // Reset players
    for (uint16_t i = 0; i < game->n_players; i++) {
      game->players[i]->cards[0] = NULL;
      game->players[i]->cards[1] = NULL;
    }

    for (size_t i = 0; i < game->n_cards; i++) {
      game->cards[i] = NULL;
    }

    // Reset deck
    table_deck_reset(game->deck);
    table_deck_shuffle(game->deck);

    game->n_cards = 0;
    game->pot = 0;

    game->state = POKER_DEAL;
    game->rounds_played++;
    break;
  }
  return true;
}
