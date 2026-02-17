#include "menu.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GAME_TITLE "- A Casino Game -"

char *log_buffer = NULL;

void menu_main_print(struct tui_ui *ui) {
  char ws[10], hs[10];
  snprintf(ws, sizeof(ws), "w: %d", ui->w);
  snprintf(hs, sizeof(hs), "h: %d", ui->h);

  tui_text(ui, 1, ui->h - 3, ws);
  tui_text(ui, 1, ui->h - 2, hs);

  if (log_buffer != NULL) {
    tui_text(ui, 1, ui->h, log_buffer);
  }

  struct tui_ascii *box = tui_ascii_box(41, 7);
  tui_centered_ascii(ui, ui->w / 2, ui->h / 2 - 1, box);
  tui_text(ui, ui->w / 2 - 18, ui->h / 2 + 1, "Press [P] to start");
  tui_text(ui, ui->w / 2 - 18, ui->h / 2 + 2, "Press [QQ] to exit");
  tui_centered_text(ui, ui->w / 2, ui->h / 2 - 1, GAME_TITLE);

  tui_ascii_free(box);
}

void menu_log(char *msg) {
  if (log_buffer == NULL) {
    log_buffer = malloc(sizeof(char) * 1024);
  }

  memset(log_buffer, 0, sizeof(char) * 1024);
  strcat(log_buffer, msg);
}

void menu_free(void) {
  free(log_buffer);
  log_buffer = NULL;
}

char *exit_msg = "Thanks for playing!\n";
void menu_set_exit_msg(char *msg) {
  if (msg != NULL) {
    exit_msg = msg;
  }
}

void menu_print_exit_msg(void) {
  printf("%s", exit_msg);
}
