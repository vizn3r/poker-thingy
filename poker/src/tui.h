#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>

struct tui_ui {
  char *buff;
  uint16_t w;
  uint16_t h;
};

struct tui_ascii {
  char **buff;
  uint16_t w;
  uint16_t h;
};

struct tui_ui *tui_init(void);

void tui_free(struct tui_ui *ui);

void tui_render(struct tui_ui *ui);

void tui_set(struct tui_ui *ui, uint16_t x, uint16_t y, char c);

void tui_clear(struct tui_ui *ui);

void tui_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text);

void tui_centered_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text);

void tui_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii);

void tui_centered_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii);

struct tui_ascii *tui_ascii_arr(char **arr, size_t len);
