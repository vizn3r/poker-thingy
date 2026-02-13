#include "table.h"
#include "tui.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
struct tui_ascii ascii_base_card = {
  .buff = (char*[]){
    "+-----+",
    "|1    |",
    "|  3  |",
    "|    5|",
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
  {'h', '3', 3},
  {'h', '4', 4},
  {'h', '5', 5},
  {'h', '6', 6},
  {'h', '7', 7},
  {'h', '8', 8},
  {'h', '9', 9},
  {'h', 't', 10},
  {'h', 'j', 10},
  {'h', 'q', 10},
  {'h', 'k', 10},
  {'h', 'a', 11},

  {'c', '2', 2},
  {'c', '3', 3},
  {'c', '4', 4},
  {'c', '5', 5},
  {'c', '6', 6},
  {'c', '7', 7},
  {'c', '8', 8},
  {'c', '9', 9},
  {'c', 't', 10},
  {'c', 'j', 10},
  {'c', 'q', 10},
  {'c', 'k', 10},
  {'c', 'a', 11},

  {'d', '2', 2},
  {'d', '3', 3},
  {'d', '4', 4},
  {'d', '5', 5},
  {'d', '6', 6},
  {'d', '7', 7},
  {'d', '8', 8},
  {'d', '9', 9},
  {'d', 't', 10},
  {'d', 'j', 10},
  {'d', 'q', 10},
  {'d', 'k', 10},
  {'d', 'a', 11},
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

void table_deck_shuffle(struct table_deck *deck) {
  for (uint16_t i = 0; i < deck->num; i++) {
    uint16_t j = rand() % deck->num;
    struct table_card tmp = deck->cards[i];
    deck->cards[i] = deck->cards[j];
    deck->cards[j] = tmp;
  }
}

struct table_card *table_deck_draw(struct table_deck *deck) {
  if (deck->num == 0)
    return NULL;

  struct table_card *card = &deck->cards[deck->num - 1];
  deck->num--;
  return card;
}

struct tui_ascii *table_card_ascii(struct table_card *card) {
  struct tui_ascii *ascii = (struct tui_ascii *)malloc(sizeof(struct tui_ascii));
  ascii->w = ascii_base_card.w;
  ascii->h = ascii_base_card.h;
  ascii->buff = (char **)malloc(sizeof(char *) * ascii->h);
  for (uint16_t i = 0; i < ascii->h; i++) {
    ascii->buff[i] = (char *)malloc(sizeof(char) * ascii->w);
    memcpy(ascii->buff[i], ascii_base_card.buff[i], sizeof(struct tui_ascii));
  }

  ascii->buff[1][1] = card->rank;
  ascii->buff[2][3] = card->suit;
  ascii->buff[3][5] = card->rank;

  return ascii;
}
