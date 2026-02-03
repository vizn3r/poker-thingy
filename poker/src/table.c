#include "table.h"
#include "tui.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
struct tui_ascii ascii_base_card = {
  .buff = (char*[]){
    "+-----+",
    "|     |",
    "|     |",
    "|     |",
    "+-----+",
  },
  .w = 7,
  .h = 5
};

const struct table_card standard_cards[TABLE_DECK_SIZE] = {
  {'s', '2', 2},
  {'s', '3', 3},
  {'s', '4', 4},
  {'s', '5', 5},
  {'s', '6', 6},
  {'s', '7', 7},
  {'s', '8', 8},
  {'s', '9', 9},
  {'s', 't', 10},
  {'s', 'j', 10},
  {'s', 'q', 10},
  {'s', 'k', 10},
  {'s', 'a', 11},

  {'h', '2', 2},
  {'s', '3', 3},
  {'s', '4', 4},
  {'s', '5', 5},
  {'h', '6', 6},
  {'s', '7', 7},
  {'s', '8', 8},
  {'s', '9', 9},
  {'h', 't', 10},
  {'s', 'j', 10},
  {'s', 'q', 10},
  {'s', 'k', 10},
  {'h', 'a', 11},

  {'c', '2', 2},
  {'s', '3', 3},
  {'s', '4', 4},
  {'s', '5', 5},
  {'c', '6', 6},
  {'s', '7', 7},
  {'s', '8', 8},
  {'s', '9', 9},
  {'c', 't', 10},
  {'s', 'j', 10},
  {'s', 'q', 10},
  {'s', 'k', 10},
  {'c', 'a', 11},

  {'s', '2', 2},
  {'s', '3', 3},
  {'s', '4', 4},
  {'s', '5', 5},
  {'s', '6', 6},
  {'s', '7', 7},
  {'s', '8', 8},
  {'s', '9', 9},
  {'s', 't', 10},
  {'s', 'j', 10},
  {'s', 'q', 10},
  {'s', 'k', 10},
  {'s', 'a', 11},
};
// clang-format on

struct table_deck *table_deck_init(uint16_t n_decks) {
  if (n_decks == 0)
    n_decks = 1;

  struct table_deck *deck =
      (struct table_deck *)malloc(sizeof(struct table_deck) * n_decks);

  deck->num = TABLE_DECK_SIZE * n_decks;
  deck->cards = (struct table_card *)malloc(sizeof(standard_cards) * n_decks);

  for (uint16_t i = 0; i < n_decks; i++)
    memcpy(&deck->cards[i * TABLE_DECK_SIZE], standard_cards, sizeof(standard_cards));

  return deck;
}
