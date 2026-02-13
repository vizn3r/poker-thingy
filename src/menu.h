#pragma once

#include "tui.h"
#ifndef GAME_TITLE
#define GAME_TITLE "A game"
#endif

void menu_main_print(struct tui_ui *ui);

void menu_log(char *msg);
