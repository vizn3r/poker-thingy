#pragma once

#include <stdint.h>

#define TABLE_DECK_SIZE 52

extern struct tui_ascii ascii_base_card;

struct table_card {
  char suit;
  char rank;
  uint16_t value;
};

struct table_deck {
  struct table_card *cards;
  uint64_t num;
};

struct table_deck *table_deck_init(uint16_t n_decks);
