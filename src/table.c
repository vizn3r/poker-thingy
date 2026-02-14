#include "table.h"
#include "tui.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// clang-format off
struct tui_ascii ascii_base_card = {
  .buff = (char *[]){
    "┌─────┐",
    "│1    │",
    "│  ♠  │",
    "│    5│",
    "└─────┘",
  },
  .w = 7,
  .h = 5
};
const struct table_card standard_cards[TABLE_DECK_SIZE] = {
  {'S', '2', 2},
  {'S', '3', 3},
  {'S', '4', 4},
  {'S', '5', 5},
  {'S', '6', 6},
  {'S', '7', 7},
  {'S', '8', 8},
  {'S', '9', 9},
  {'S', 'T', 10},
  {'S', 'J', 10},
  {'S', 'Q', 10},
  {'S', 'K', 10},
  {'S', 'A', 11},

  {'H', '2', 2},
  {'H', '3', 3},
  {'H', '4', 4},
  {'H', '5', 5},
  {'H', '6', 6},
  {'H', '7', 7},
  {'H', '8', 8},
  {'H', '9', 9},
  {'H', 'T', 10},
  {'H', 'J', 10},
  {'H', 'Q', 10},
  {'H', 'K', 10},
  {'H', 'A', 11},

  {'C', '2', 2},
  {'C', '3', 3},
  {'C', '4', 4},
  {'C', '5', 5},
  {'C', '6', 6},
  {'C', '7', 7},
  {'C', '8', 8},
  {'C', '9', 9},
  {'C', 'T', 10},
  {'C', 'J', 10},
  {'C', 'Q', 10},
  {'C', 'K', 10},
  {'C', 'A', 11},

  {'D', '2', 2},
  {'D', '3', 3},
  {'D', '4', 4},
  {'D', '5', 5},
  {'D', '6', 6},
  {'D', '7', 7},
  {'D', '8', 8},
  {'D', '9', 9},
  {'D', 'T', 10},
  {'D', 'J', 10},
  {'D', 'Q', 10},
  {'D', 'K', 10},
  {'D', 'A', 11},
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

  table_deck_shuffle(deck);

  return deck;
}

void table_deck_shuffle(struct table_deck *deck) {
  srand(time(NULL));
  for (uint16_t i = 0; i < deck->num; i++) {
    uint16_t j = i + rand() % (deck->num - i);
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
    size_t len = strlen(ascii_base_card.buff[i]) + 1;
    ascii->buff[i] = (char *)malloc(len);
    memcpy(ascii->buff[i], ascii_base_card.buff[i], len);
  }

  ascii->buff[1][3] = card->rank;
  ascii->buff[3][7] = card->rank;

  const char *suit;
  switch (card->suit) {
  case 's':
    suit = "♠";
    break;
  case 'h':
    suit = "♥";
    break;
  case 'c':
    suit = "♣";
    break;
  case 'd':
    suit = "♦";
    break;
  }
  memcpy(&ascii->buff[2][5], suit, 3);

  return ascii;
}
