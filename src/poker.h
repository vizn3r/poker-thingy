#pragma once

#include "stdbool.h"
#include "table.h"
#include "tui.h"
#include <stdint.h>

struct poker_board {
  struct table_card cards[5];
  uint8_t n_cards;
};

enum poker_player_role {
  PLAYER_DEALER,
  PLAYER_SMALL_BLIND,
  PLAYER_BIG_BLIND,
  PLAYER_NORMAL
};

struct poker_player {
  struct table_card cards[2];
  uint16_t id;
  char *name;
  uint64_t money;
  enum poker_player_role role;
};

enum poker_game_state {
  POKER_SMALL_BLIND,
  POKER_BIG_BLIND,
  POKER_PREFLOP,
  POKER_FLOP,
  POKER_TURN,
  POKER_RIVER,
  POKER_SHOW_CARDS,
  POKER_ROUND_END
};

struct poker_game {
  struct poker_board *board;
  struct poker_player **players;
  uint16_t n_players;
  uint64_t pot;
  enum poker_game_state state;
  uint32_t rounds_played;
};

struct poker_game *poker_init(void);

void poker_free(struct poker_game *game);

bool poker_play(struct tui_ui *ui, struct poker_game *game);
