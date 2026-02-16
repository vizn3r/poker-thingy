#pragma once

#include "tui.h"

void menu_main_print(struct tui_ui *ui);

void menu_log(char *msg);

void menu_free(void);

void menu_set_exit_msg(char *msg);

void menu_print_exit_msg(void);
