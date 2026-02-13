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

void table_deck_shuffle(struct table_deck *deck);

struct table_card *table_deck_draw(struct table_deck *deck);

struct tui_ascii *table_card_ascii(struct table_card *card);
