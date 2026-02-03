#include "tui.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct tui_ui *tui_init(void) {
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

  struct tui_ui *ui = (struct tui_ui *)malloc(sizeof(struct tui_ui));

  ui->h = ws.ws_row;
  ui->w = ws.ws_col;

  ui->buff = (char *)malloc(sizeof(char) * ui->h * ui->w);
  memset(ui->buff, ' ', ui->h * ui->w);

  return ui;
}

void tui_free(struct tui_ui *ui) {
  printf("\033[H\033[J");
  fflush(stdout);
  free(ui->buff);
  free(ui);
}

void tui_render(struct tui_ui *ui) {
  printf("\033[H");
  fwrite(ui->buff, sizeof(char), ui->h * ui->w, stdout);
  fflush(stdout);
}

void tui_set(struct tui_ui *ui, uint16_t x, uint16_t y, char c) {
  ui->buff[y * ui->w + x] = c;
}

void tui_clear(struct tui_ui *ui) { memset(ui->buff, ' ', ui->h * ui->w); }

void tui_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text) {
  uint16_t i = 0;
  while (text[i] != '\0') {
    ui->buff[y * ui->w + x + i] = text[i];
    i++;
  }
}

void tui_centered_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text) {
  size_t len = strlen(text);
  uint16_t i = 0;
  while (text[i] != '\0') {
    ui->buff[y * ui->w + x + i - len / 2] = text[i];
    i++;
  }
}

void tui_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii) {
  for (uint16_t i = 0; i < ascii->h; i++) {
    size_t len = strlen(ascii->buff[i]);
    memcpy(&ui->buff[(y + i) * ui->w + x], ascii->buff[i], len);
  }
}

void tui_centered_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii) {
  for (uint16_t i = 0; i < ascii->h; i++) {
    size_t len = strlen(ascii->buff[i]);
    memcpy(&ui->buff[(y + i) * ui->w + x - ascii->w / 2], ascii->buff[i], len);
  }
}
