#pragma once

#include "stdbool.h"
#include "table.h"
#include "tui.h"
#include <stdint.h>

enum poker_player_role {
  PLAYER_DEALER,
  PLAYER_SMALL_BLIND,
  PLAYER_BIG_BLIND,
  PLAYER_NORMAL
};

enum poker_player_action {
  PLAYER_ACTION_NONE = 1 << 0,
  PLAYER_ACTION_FOLD = 1 << 1,
  PLAYER_ACTION_CHECK_CALL = 1 << 2,
  PLAYER_ACTION_BET = 1 << 3,
  PLAYER_ACTION_RAISE = 1 << 4,
  PLAYER_ACTION_ALL_IN = 1 << 5,

  PLAYER_ACTION_MAX = 1 << 6 // For looping purposes
};

struct poker_player {
  struct table_card *cards[2];
  char *name;
  uint64_t money;
  uint64_t bet;
  bool folded;
  bool all_in;
  bool show_cards;
  enum poker_player_role role;
  enum poker_player_action action;
  enum poker_player_action possible_actions;
};

enum poker_game_state {
  POKER_DEAL,
  POKER_SMALL_BLIND,
  POKER_BIG_BLIND,
  POKER_PREFLOP,
  POKER_FLOP,
  POKER_TURN,
  POKER_RIVER,
  POKER_SHOW_CARDS,
  POKER_ROUND_END,

  POKER_STATE_MAX
};

struct poker_game {
  // Game state
  struct poker_board *board;
  enum poker_game_state state;
  uint32_t rounds_played;

  // 1st person player - main player
  struct poker_player *main_player;

  // Cards
  struct table_deck *deck;
  struct table_deck *discard;
  struct table_card *cards[5];
  size_t n_cards;

  // Players
  uint16_t dealer; // First player to get cards
  struct poker_player **players;
  size_t n_players;
  uint16_t last_aggressor; // Last player to bet/raise

  // Money
  uint64_t pot;
  uint64_t current_bet;
  uint64_t *side_pots;
  size_t n_side_pots;
  uint64_t small_blind;
  uint64_t big_blind;
};

struct poker_game *poker_init(void);

void poker_free(struct poker_game *game);

void poker_play(struct tui_ui *ui, struct poker_game *game);

void poker_input(struct poker_game *game, int key);

void poker_display(struct tui_ui *ui, struct poker_game *game);
