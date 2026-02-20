#include "poker.h"
#include "input.h"
#include "table.h"
#include "tui.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define NUM_PLAYERS 5
#define STARTING_BAL 1000
#define SMALL_BLIND 1
#define BIG_BLIND 2

// clang-format off
const char *poker_states[] = {
    " Deal ",
    " Small Blind ",
    " Big Blind ",
    " Preflop ",
    " Flop ",
    " Turn ",
    " River ",
    " Showdown ",
    " Round End ",
};

const char *poker_actions[] = {
    "[Any] to continue",
    "[FF]old",
    " [C]heck/Call",
    " [B]et ",      // Perfectly padded, just so all the strings are odd length and appear centered
    "[R]aise",
    " [AA]ll In!",    // Perfectly padded, just so all the strings are odd length for centering
};

const char *poker_hands[] = {
    "High Card",
    "One Pair",
    "Two Pair",
    "Three of a Kind",
    "Straight",
    "Flush",
    "Full House",
    "Four of a Kind",
    "Straight Flush",
    "Royal Flush"
};

const struct tui_ascii_box ascii_box_player = {
  .corner_br = "╯",
  .corner_tr = "╮",
  .corner_tl = "╭",
  .corner_bl = "╰",
  .horiz = "─",
  .vert = "│",
};

const struct tui_ascii_box ascii_box_board = {
  .corner_br = "╯",
  .corner_tr = "╮",
  .corner_tl = "╭",
  .corner_bl = "╰",
  .horiz = "┄",
  .vert = "┊",
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
    game->players[i]->possible_actions = PLAYER_ACTION_NONE;
    game->players[i]->action = PLAYER_ACTION_NONE;
    game->players[i]->folded = false;
    game->players[i]->all_in = false;
    game->players[i]->show_cards = false;
    game->players[i]->name = "Player";
    game->players[i]->hand_rank = 0;
    game->players[i]->cards[0] = NULL;
    game->players[i]->cards[1] = NULL;
    game->players[i]->bet = 0;
  }

  game->main_player = game->players[0];
  if (game->main_player == NULL) {
    return NULL;
  }

  game->pot = 0;
  game->current_bet = 0;
  game->last_aggressor = 0;
  game->dealer = 0;
  game->state = POKER_DEAL;
  game->n_cards = 0;
  game->rounds_played = 0;
  game->winner = 0;

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
void poker_display(struct tui_ui *ui, struct poker_game *game) {
  if (game == NULL) {
    tui_text(ui, 1, 1, "No game to display");
    return;
  }
  struct poker_player *mp = game->main_player;

  // Show main player's status
  uint8_t mp_x = ui->w / 2;
  uint8_t mp_y = tui_gy(ui, 6);

  struct tui_ascii *mp_cards = tui_ascii_custom_box(17, 7, ascii_box_player);
  tui_centered_ascii(ui, mp_x, mp_y, mp_cards);
  tui_centered_text(ui, mp_x, mp_y - 3, mp->name);
  char role[16] = "";
  switch (mp->role) {
  case PLAYER_BIG_BLIND:
    strcat(role, "B-B");
    break;
  case PLAYER_SMALL_BLIND:
    strcat(role, "S-B");
    break;
  case PLAYER_DEALER:
    strcat(role, "DLR");
    break;
  default:
    break;
  }
  tui_centered_text(ui, mp_x, mp_y + 4, role);
  char chips[32] = "";
  snprintf(chips, 32, "( $%" PRId64 " )", mp->money);
  tui_centered_text(ui, mp_x, mp_y + 3, chips);
  if (game->state >= POKER_DEAL)
    tui_centered_text(ui, mp_x, mp_y - 4, (char *)poker_hands[mp->hand_rank]);
  if (game->state > POKER_SHOW_CARDS && game->winner == 0)
    tui_centered_text(ui, mp_x, mp_y - 5, "Winner!");

  if (mp->cards[0] != NULL && mp->cards[1] != NULL) {
    struct tui_ascii *mp_card1 = table_card_ascii(mp->cards[0]);
    struct tui_ascii *mp_card2 = table_card_ascii(mp->cards[1]);
    tui_centered_ascii(ui, mp_x - 3, mp_y, mp_card1);
    tui_centered_ascii(ui, mp_x + 3, mp_y, mp_card2);
    tui_ascii_free(mp_card1);
    tui_ascii_free(mp_card2);
  }
  tui_ascii_free(mp_cards);

  // Show other player's status
  uint16_t player_base_y = tui_gy(ui, 2);
  int n = (int)game->n_players;
  int box_w = 17;
  int gap = 1;
  int spacing = box_w + gap;
  int total_w = (n - 1) * spacing - gap;
  int start_x = ui->w / 2 - total_w / 2;

  struct tui_ascii *player_box = tui_ascii_custom_box(box_w, 7, ascii_box_player);
  for (int i = 1; i < n; i++) {
    uint16_t player_base_x = start_x + box_w / 2 + (i - 1) * spacing;
    tui_centered_ascii(ui, player_base_x, player_base_y, player_box);
    tui_centered_text(ui, player_base_x, player_base_y - 3, game->players[i]->name);
    char role[16] = "";
    switch (game->players[i]->role) {
    case PLAYER_BIG_BLIND:
      strcat(role, "B-B");
      break;
    case PLAYER_SMALL_BLIND:
      strcat(role, "S-B");
      break;
    case PLAYER_DEALER:
      strcat(role, "DLR");
      break;
    default:
      break;
    }
    tui_centered_text(ui, player_base_x, player_base_y + 4, role);
    char chips[32] = "";
    snprintf(chips, 32, "( $%" PRId64 " )", game->players[i]->money);
    tui_centered_text(ui, player_base_x, player_base_y + 3, chips);
    if (game->state >= POKER_SHOW_CARDS)
      tui_centered_text(ui, player_base_x, player_base_y - 4, (char *)poker_hands[game->players[i]->hand_rank]);
    if (game->state > POKER_SHOW_CARDS && game->winner == i)
      tui_centered_text(ui, player_base_x, player_base_y - 5, "Winner!");

    if (game->players[i]->cards[0] != NULL && game->players[i]->cards[1] != NULL) {
      struct tui_ascii *card1 = NULL;
      struct tui_ascii *card2 = NULL;
      if (game->state >= POKER_SHOW_CARDS) {
        card1 = table_card_ascii(game->players[i]->cards[0]);
        card2 = table_card_ascii(game->players[i]->cards[1]);
      } else {
        card1 = table_card_back_ascii();
        card2 = table_card_back_ascii();
      }
      tui_centered_ascii(ui, player_base_x - 3, player_base_y, card2);
      tui_centered_ascii(ui, player_base_x + 3, player_base_y, card1);
      tui_centered_ascii(ui, player_base_x - 3, player_base_y, card1);
      tui_centered_ascii(ui, player_base_x + 3, player_base_y, card2);
      tui_ascii_free(card1);
      tui_ascii_free(card2);
    }
  }
  tui_ascii_free(player_box);

  // Show board
  uint16_t board_x = tui_gx(ui, 4);
  uint16_t board_y = ui->h / 2;
  struct tui_ascii *board = tui_ascii_custom_box(43, 7, ascii_box_board);
  tui_centered_ascii(ui, board_x, board_y, board);
  for (size_t i = 0; i < game->n_cards; i++) {
    struct tui_ascii *card = table_card_ascii(game->cards[i]);
    tui_centered_ascii(ui, board_x + (i - 2) * 8, board_y, card);
    tui_ascii_free(card);
  }
  tui_ascii_free(board);

  // Show current poker state
  tui_centered_text(ui, board_x - 43 / 4, board_y - 3, (char *)poker_states[game->state]);
  char rounds_played[32] = "";
  snprintf(rounds_played, 32, " Round %d ", game->rounds_played);
  tui_centered_text(ui, board_x + 43 / 4, board_y - 3, rounds_played);
  char pot[32] = "";
  snprintf(pot, 32, "( $%" PRIu64 " )", game->pot);
  tui_centered_text(ui, board_x, board_y + 3, pot);

  // Show main player's actions
  char actions[128] = "";
  uint16_t action_id = 0;
  for (uint32_t action = PLAYER_ACTION_NONE; action < PLAYER_ACTION_MAX; action <<= 1, action_id++) {
    if (mp->possible_actions & action) {
      strcat(actions, (char *)poker_actions[action_id]);
    }
  }
  tui_centered_text(ui, ui->w / 2, ui->h - 2, actions);
}

// Returns 0 - 9 based on the best combination
uint8_t poker_player_eval_hand(struct poker_game *game, size_t player_id) {
  struct poker_player *player = game->players[player_id];
  struct table_card *cards[7] = {0};
  cards[0] = player->cards[0];
  cards[1] = player->cards[1];
  for (size_t i = 0; i < game->n_cards; i++)
    cards[2 + i] = game->cards[i];

  uint16_t suit_mask[4] = {0};

  for (uint8_t i = 0; i < 2 + game->n_cards; i++) {
    uint8_t suit;
    switch (cards[i]->suit) {
    case 'h':
    case 'H': suit = 0; break;
    case 'c':
    case 'C': suit = 1; break;
    case 'd':
    case 'D': suit = 2; break;
    default: suit = 3; break;
    }
    suit_mask[suit] |= (1 << cards[i]->value);
  }

  uint16_t all = suit_mask[0] | suit_mask[1] | suit_mask[2] | suit_mask[3];

  for (uint8_t s = 0; s < 4; s++)
    for (uint8_t r = 2; r <= 10; r++)
      if ((suit_mask[s] >> r & 0x1F) == 0x1F)
        return (r == 10) ? 9 : 8; // royal flush, straight flush

  uint8_t pattern[7] = {0};
  uint8_t k = 0;
  for (uint8_t r = 2; r <= 14; r++) {
    uint8_t count = 0;
    for (uint8_t s = 0; s < 4; s++)
      count += (suit_mask[s] >> r) & 1;
    if (count) pattern[k++] = count;
  }

  // sort descending
  for (uint8_t a = 0; a < k - 1; a++)
    for (uint8_t b = a + 1; b < k; b++)
      if (pattern[b] > pattern[a]) {
        uint8_t tmp = pattern[a];
        pattern[a] = pattern[b];
        pattern[b] = tmp;
      }

  if (pattern[0] == 4) return 7;                    // four of a kind
  if (pattern[0] == 3 && pattern[1] == 2) return 6; // full house
  for (uint8_t s = 0; s < 4; s++)
    if (__builtin_popcount(suit_mask[s]) >= 5) return 5; // flush
  for (uint8_t r = 2; r <= 10; r++)
    if ((all >> r & 0x1F) == 0x1F) return 4;        // straight
  if (pattern[0] == 3) return 3;                    // three of a kind
  if (pattern[0] == 2 && pattern[1] == 2) return 2; // two pair
  if (pattern[0] == 2) return 1;                    // pair
  return 0;                                         // high card
}

void poker_player_draw_resolve(struct poker_game *game, uint16_t *player_ids, size_t n_players) {
  uint16_t winner = player_ids[0];

  for (size_t i = 1; i < n_players; i++) {
    struct poker_player *a = game->players[winner];
    struct poker_player *b = game->players[player_ids[i]];

    if (a->hand_rank == 4 || a->hand_rank == 8) {
      uint16_t all_a = 0, all_b = 0;
      all_a |= (1 << a->cards[0]->value) | (1 << a->cards[1]->value);
      all_b |= (1 << b->cards[0]->value) | (1 << b->cards[1]->value);
      for (size_t j = 0; j < game->n_cards; j++) {
        all_a |= (1 << game->cards[j]->value);
        all_b |= (1 << game->cards[j]->value);
      }

      uint8_t high_a = 0, high_b = 0, consec = 0;
      for (uint8_t r = 14; r >= 2; r--) {
        if ((all_a >> r) & 1) consec++;
        else
          consec = 0;
        if (consec >= 5) {
          high_a = r + 4;
          break;
        }
      }
      consec = 0;
      for (uint8_t r = 14; r >= 2; r--) {
        if ((all_b >> r) & 1) consec++;
        else
          consec = 0;
        if (consec >= 5) {
          high_b = r + 4;
          break;
        }
      }

      if (high_b > high_a) winner = player_ids[i];
      if (high_b != high_a) continue;
    }

    uint16_t a_high = a->cards[0]->value > a->cards[1]->value ? a->cards[0]->value : a->cards[1]->value;
    uint16_t b_high = b->cards[0]->value > b->cards[1]->value ? b->cards[0]->value : b->cards[1]->value;

    if (b_high > a_high) {
      winner = player_ids[i];
      continue;
    }
    if (b_high < a_high) continue;

    uint16_t a_low = a->cards[0]->value < a->cards[1]->value ? a->cards[0]->value : a->cards[1]->value;
    uint16_t b_low = b->cards[0]->value < b->cards[1]->value ? b->cards[0]->value : b->cards[1]->value;

    if (b_low > a_low) winner = player_ids[i];
  }

  game->winner = winner;
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
    player->possible_actions = PLAYER_ACTION_NONE;
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

// This is to prevent missinputs
bool poker_player_do_action(struct poker_game *game, size_t player_id) {
  struct poker_player *player = game->players[player_id];

  // Check the action is actually available to the player
  if (!(player->possible_actions & player->action))
    return false;

  switch (player->action) {
  case PLAYER_ACTION_FOLD:
    player->folded = true;
    break;
  case PLAYER_ACTION_CHECK_CALL:
    if (player->bet != game->current_bet) {
      int tmp = game->current_bet - player->bet;
      game->pot += tmp;
      player->money -= tmp;
      player->bet += tmp;
    }
    break;
  case PLAYER_ACTION_BET:
    break;
  case PLAYER_ACTION_RAISE:
    break;
  case PLAYER_ACTION_ALL_IN:
    player->all_in = true;
    break;
  default:
    break;
  }

  player->action = PLAYER_ACTION_NONE;
  return true;
}

// Returns true if player performed an action
bool fold_latch = false;
bool all_in_latch = false;
bool poker_handle_player_input(struct poker_game *game, size_t player_id) {
  struct poker_player *player = game->players[player_id];

  if (game->state < POKER_PREFLOP && input_get_key() != -1) {
    input_consume();
    player->possible_actions = PLAYER_ACTION_NONE;
    return true;
  }

  switch (input_get_key()) {
  case 'q':
  case 'Q':
  case INPUT_KEY_ESC:
    input_consume();
    return false;
  case 'f':
  case 'F':
    if (!fold_latch) {
      fold_latch = true;
      return false;
    }
    player->action = PLAYER_ACTION_FOLD;
    break;
  case 'c':
  case 'C':
    player->action = PLAYER_ACTION_CHECK_CALL;
    break;
  case 'b':
  case 'B':
    player->action = PLAYER_ACTION_BET;
    break;
  case 'r':
  case 'R':
    player->action = PLAYER_ACTION_RAISE;
    break;
  case 'a':
  case 'A':
    if (!all_in_latch) {
      all_in_latch = true;
      return false;
    }
    player->action = PLAYER_ACTION_ALL_IN;
    break;
  case -1:
    return false;
  default:
    if (player->possible_actions == PLAYER_ACTION_NONE) {
      input_consume();
      return true;
    }
    return false;
  }

  fold_latch = false;
  all_in_latch = false;

  input_consume();
  return poker_player_do_action(game, player_id);
}

bool next = false;
void poker_play(struct tui_ui *ui, struct poker_game *game) {
  (void)ui;
  size_t sb = (game->dealer + 1) % game->n_players;
  size_t bb = (game->dealer + 2) % game->n_players;

  // The poker state machine
top:
  switch (game->state) {

  // Players are dealt
  case POKER_DEAL:
    if (game->players[0]->cards[0] == NULL) {
      for (size_t i = 0; i < game->n_players; i++) {
        game->players[i]->role = PLAYER_NORMAL;
        game->players[i]->possible_actions = PLAYER_ACTION_NONE;

        game->players[i]->cards[0] = table_deck_draw(game->deck);
        game->players[i]->cards[1] = table_deck_draw(game->deck);
        if (game->players[i]->cards[0] == NULL || game->players[i]->cards[1] == NULL) {
          tui_text(ui, 1, ui->h - 3, "Error: Could not initialize poker game");
          return;
        }
      }

      game->players[game->dealer]->role = PLAYER_DEALER;
      game->players[sb]->role = PLAYER_SMALL_BLIND;
      game->players[bb]->role = PLAYER_BIG_BLIND;

      game->last_aggressor = bb; // BB is by default the first last aggressor
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_SMALL_BLIND;
      goto top;
    }

    break;

  // Small blind player pays
  case POKER_SMALL_BLIND:
    if (game->pot == 0) {
      game->pot += game->small_blind;
      game->players[sb]->money -= game->small_blind;
      game->players[sb]->bet = game->small_blind;
      game->current_bet = game->small_blind;
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_BIG_BLIND;
      goto top;
    }
    break;

  // Big blind player pays
  case POKER_BIG_BLIND:
    if (game->pot == game->small_blind) {
      game->pot += game->big_blind;
      game->players[bb]->money -= game->big_blind;
      game->players[bb]->bet = game->big_blind;
      game->current_bet = game->big_blind;
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_PREFLOP;
      goto top;
    }
    break;

  // Players call/check/fold/raise
  case POKER_PREFLOP:
    if (game->pot < game->big_blind) {
      // For now, alll non-main players will call
      // TODO: make proper ordered player actions
      for (size_t i = 1; i < game->n_players; i++) {
        game->players[i]->action = PLAYER_ACTION_CHECK_CALL;
        poker_player_do_action(game, i);
      }
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_FLOP;
      goto top;
    }
    break;

  // First 3 cards are dealt on the board, players check/fold/raise/call
  case POKER_FLOP:
    if (game->n_cards == 0) {
      for (uint8_t i = 0; i < 3; i++) {
        game->cards[i] = table_deck_draw(game->deck);
      }
      game->n_cards = 3;
      // For now, alll non-main players will call
      // TODO: make proper ordered player actions
      for (size_t i = 1; i < game->n_players; i++) {
        game->players[i]->action = PLAYER_ACTION_CHECK_CALL;
        poker_player_do_action(game, i);
      }
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_TURN;
      goto top;
    }
    break;

  // 4th card is dealt on the board, players check/fold/raise/call
  case POKER_TURN:
    if (game->n_cards == 3) {
      if (game->n_cards == 3) {
        game->cards[3] = table_deck_draw(game->deck);
      }
      game->n_cards = 4;
      // For now, alll non-main players will call
      // TODO: make proper ordered player actions
      for (size_t i = 1; i < game->n_players; i++) {
        game->players[i]->action = PLAYER_ACTION_CHECK_CALL;
        poker_player_do_action(game, i);
      }
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_RIVER;
      goto top;
    }
    break;

  // 5th card is dealt on the board, players check/fold/raise/call
  case POKER_RIVER:
    if (game->n_cards == 4) {
      if (game->n_cards == 4) {
        game->cards[4] = table_deck_draw(game->deck);
      }
      game->n_cards = 5;
      // For now, alll non-main players will call
      // TODO: make proper ordered player actions
      for (size_t i = 1; i < game->n_players; i++) {
        game->players[i]->action = PLAYER_ACTION_CHECK_CALL;
        poker_player_do_action(game, i);
      }
      return;
    }

    for (size_t i = 0; i < game->n_players; i++) {
      game->players[i]->hand_rank = poker_player_eval_hand(game, i);
      poker_check_player_possible_actions(game, i);
    }
    poker_check_player_possible_actions(game, 0);
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_SHOW_CARDS;
      goto top;
    }
    break;

  // Evaulation of the board, players get payed out
  case POKER_SHOW_CARDS:
    if (game->pot > 0) {
      uint16_t winning_players[game->n_players];
      size_t n_winning_players = 0;
      for (size_t i = 0; i < game->n_players; i++) {
        game->players[i]->hand_rank = poker_player_eval_hand(game, i);
        if (game->players[i]->hand_rank > game->players[game->winner]->hand_rank)
          game->winner = i;
      }

      for (size_t i = 0; i < game->n_players; i++) {
        if (game->players[i]->hand_rank == game->players[game->winner]->hand_rank) {
          winning_players[n_winning_players] = i;
          n_winning_players++;
        }
      }

      if (n_winning_players > 1)
        poker_player_draw_resolve(game, winning_players, n_winning_players);
      else
        game->winner = winning_players[0];

      game->players[game->winner]->money += game->pot;
      game->pot = 0;
      return;
    }

    poker_check_player_possible_actions(game, 0);
    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_ROUND_END;
      goto top;
    }
    break;

  // End of the round, reset deck, dealer, and players
  case POKER_ROUND_END:
    if (game->n_cards > 0) {
      // Advance to next dealer
      game->dealer = (game->dealer + 1) % game->n_players;

      // Reset players
      for (size_t i = 0; i < game->n_players; i++) {
        game->players[i]->cards[0] = NULL;
        game->players[i]->cards[1] = NULL;
        game->players[i]->folded = false;
        game->players[i]->all_in = false;
        game->players[i]->show_cards = false;
        game->players[i]->bet = 0;
      }

      for (size_t i = 0; i < game->n_cards; i++) {
        game->cards[i] = NULL;
      }

      // Reset deck
      table_deck_reset(game->deck);
      table_deck_shuffle(game->deck);

      game->n_cards = 0;
      game->winner = 0;
      return;
    }

    next = poker_handle_player_input(game, 0);

    if (next) {
      game->state = POKER_DEAL;
      game->rounds_played++;
      goto top;
    }
    break;
  default:
    break;
  }

  return;
}
