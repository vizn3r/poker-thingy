#include "tui.h"
#include "menu.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define TUI_SHOW_GRID 0
#define TUI_GRID_X 8
#define TUI_GRID_Y 8

struct tui_ui *tui_init(void) {
  printf("\033[?1049h");
  printf("\033[?1000l\033[?1003l");

  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  struct tui_ui *ui = (struct tui_ui *)malloc(sizeof(struct tui_ui));

  ui->h = ws.ws_row;
  ui->w = ws.ws_col;

  ui->buff = (char *)calloc(ui->h * ui->w * 4, sizeof(char)); // 4 bytes per char
  ui->cells = (struct tui_cell *)malloc(sizeof(struct tui_cell) * ui->h * ui->w);
  for (uint32_t i = 0; i < ui->h * ui->w; i++) {
    ui->cells[i].c[0] = ' ';
    ui->cells[i].size = 1;
  }

  return ui;
}

void tui_free(struct tui_ui *ui) {
  if (ui == NULL || ui->buff == NULL)
    return;
  free(ui->buff);
  free(ui->cells);
  free(ui);
  ui = NULL;
  printf("\033[H\033[J");
  printf("\033[?1049l");
  printf("\033[?25h");
  fflush(stdout);
}

void tui_show_grid(struct tui_ui *ui) {
  // Vertical lines - divide width into 8ths
  for (uint16_t i = 1; i < TUI_GRID_X; i++) {
    uint16_t x = (ui->w / TUI_GRID_X) * i;
    char label[2];
    label[0] = '0' + i;
    label[1] = '\0';
    tui_text(ui, x, 0, label);
    for (uint16_t y = 1; y < ui->h; y++) {
      tui_text(ui, x, y, "│");
    }
  }

  // Horizontal lines - divide height into 8ths
  for (uint16_t i = 1; i < TUI_GRID_Y; i++) {
    uint16_t y = (ui->h / TUI_GRID_Y) * i;
    char label[2];
    label[0] = '0' + i;
    label[1] = '\0';
    tui_text(ui, 0, y, label);
    for (uint16_t x = 1; x < ui->w; x++) {
      tui_text(ui, x, y, "─");
    }
  }
}

uint16_t tui_gx(struct tui_ui *ui, uint16_t x) { return x * ui->w / TUI_GRID_X; }
uint16_t tui_gy(struct tui_ui *ui, uint16_t y) { return y * ui->h / TUI_GRID_Y; }

void tui_render(struct tui_ui *ui) {
  printf("\033[?25l\033[1;1H");
  fflush(stdout);
  size_t cursor = 0;
  memset(ui->buff, 0, ui->h * ui->w * 4);

  if (ui->w < 130 && ui->h < 30) {
    tui_clear(ui);
    tui_text(ui, 1, 1, "Terminal too small!");
    tui_text(ui, 1, 2, "Press [QQ] to exit or resize your terminal!");
  }

  if (ui->w > 200 && ui->h > 50) {
    tui_clear(ui);
    tui_text(ui, 1, 1, "Terminal too large!");
    tui_text(ui, 1, 2, "Press [QQ] to exit or resize your terminal!");
  }

  for (uint32_t i = 0; i < ui->h * ui->w; i++) {
    memcpy(&ui->buff[cursor], ui->cells[i].c, ui->cells[i].size);
    cursor += ui->cells[i].size;
    if ((i + 1) % ui->w == 0 && i != (uint32_t)(ui->h * ui->w) - 1) {
      ui->buff[cursor] = '\n';
      cursor++;
    }
  }

  fwrite(ui->buff, sizeof(char), cursor, stdout);
  fflush(stdout);
}

void tui_clear(struct tui_ui *ui) {
  if (ui == NULL)
    return;
  for (uint32_t i = 0; i < ui->h * ui->w; i++) {
    ui->cells[i].c[0] = ' ';
    ui->cells[i].size = 1;
  }
  if (TUI_SHOW_GRID) {
    tui_show_grid(ui);
  }
}
size_t utf8_char_size(char c) {
  if ((c & 0x80) == 0x00)
    return 1;
  if ((c & 0xE0) == 0xC0)
    return 2;
  if ((c & 0xF0) == 0xE0)
    return 3;
  if ((c & 0xF8) == 0xF0)
    return 4;
  return 1; // continuation byte, shouldn't hit this
}

size_t utf8_strlen(char *text) {
  size_t len = 0;
  size_t i = 0;
  while (text[i] != '\0') {
    i += utf8_char_size(text[i]);
    len++;
  }
  return len;
}

void tui_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text) {
  if (y >= ui->h)
    return;
  uint16_t i = 0;
  uint16_t col = 0;
  while (text[i] != '\0') {
    if (x + col >= ui->w)
      return;
    size_t size = utf8_char_size(text[i]);
    memcpy(ui->cells[y * ui->w + x + col].c, &text[i], size);
    ui->cells[y * ui->w + x + col].size = size;
    i += size;
    col++;
  }
}

void tui_centered_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text) {
  if (y >= ui->h)
    return;
  size_t len = utf8_strlen(text);
  uint16_t i = 0;
  uint16_t col = 0;
  while (text[i] != '\0') {
    if (x + col >= ui->w)
      return;
    size_t size = utf8_char_size(text[i]);
    memcpy(ui->cells[y * ui->w + x + col - len / 2].c, &text[i], size);
    ui->cells[y * ui->w + x + col - len / 2].size = size;
    i += size;
    col++;
  }
}

void tui_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii) {
  if (y >= ui->h)
    return;
  for (uint16_t row = 0; row < ascii->h; row++) {
    uint16_t i = 0;
    uint16_t col = 0;
    while (ascii->buff[row][i] != '\0') {
      if (x + col >= ui->w)
        return;
      size_t size = utf8_char_size(ascii->buff[row][i]);
      memcpy(ui->cells[(y + row) * ui->w + x + col].c, &ascii->buff[row][i], size);
      ui->cells[(y + row) * ui->w + x + col].size = size;
      i += size;
      col++;
    }
  }
}

void tui_centered_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii) {
  for (uint16_t row = 0; row < ascii->h; row++) {
    int16_t actual_y = y + row - ascii->h / 2;
    if (actual_y < 0 || actual_y >= ui->h)
      continue;

    uint16_t i = 0;
    uint16_t col = 0;
    while (ascii->buff[row][i] != '\0') {
      int16_t actual_x = x + col - ascii->w / 2;
      if (actual_x < 0 || actual_x >= ui->w)
        break;

      size_t size = utf8_char_size(ascii->buff[row][i]);
      memcpy(ui->cells[actual_y * ui->w + actual_x].c, &ascii->buff[row][i], size);
      ui->cells[actual_y * ui->w + actual_x].size = size;
      i += size;
      col++;
    }
  }
}

#define ASCII_BOX_VERT "║"
#define ASCII_BOX_HORIZ "═"
#define ASCII_BOX_CORNER_TL "╔"
#define ASCII_BOX_CORNER_TR "╗"
#define ASCII_BOX_CORNER_BL "╚"
#define ASCII_BOX_CORNER_BR "╝"

struct tui_ascii *tui_ascii_box(size_t w, size_t h) {
  struct tui_ascii *ascii = (struct tui_ascii *)malloc(sizeof(struct tui_ascii));
  ascii->w = w;
  ascii->h = h;
  ascii->buff = (char **)malloc(sizeof(char *) * h);

  for (uint16_t i = 0; i < h; i++) {
    ascii->buff[i] = (char *)malloc(w * 4 + 1);
    size_t cursor = 0;

    for (uint16_t j = 0; j < w; j++) {
      const char *ch;
      if (i == 0 && j == 0)
        ch = ASCII_BOX_CORNER_TL;
      else if (i == 0 && j == w - 1)
        ch = ASCII_BOX_CORNER_TR;
      else if (i == h - 1 && j == 0)
        ch = ASCII_BOX_CORNER_BL;
      else if (i == h - 1 && j == w - 1)
        ch = ASCII_BOX_CORNER_BR;
      else if (i == 0 || i == h - 1)
        ch = ASCII_BOX_HORIZ;
      else if (j == 0 || j == w - 1)
        ch = ASCII_BOX_VERT;
      else
        ch = " ";

      size_t len = strlen(ch);
      memcpy(&ascii->buff[i][cursor], ch, len);
      cursor += len;
    }

    ascii->buff[i][cursor] = '\0';
  }

  return ascii;
}

struct tui_ascii *tui_ascii_custom_box(size_t w, size_t h, struct tui_ascii_box box) {
  struct tui_ascii *ascii = (struct tui_ascii *)malloc(sizeof(struct tui_ascii));
  ascii->w = w;
  ascii->h = h;
  ascii->buff = (char **)malloc(sizeof(char *) * h);

  for (uint16_t i = 0; i < h; i++) {
    ascii->buff[i] = (char *)malloc(w * 4 + 1);
    size_t cursor = 0;

    for (uint16_t j = 0; j < w; j++) {
      const char *ch;
      if (i == 0 && j == 0)
        ch = box.corner_tl;
      else if (i == 0 && j == w - 1)
        ch = box.corner_tr;
      else if (i == h - 1 && j == 0)
        ch = box.corner_bl;
      else if (i == h - 1 && j == w - 1)
        ch = box.corner_br;
      else if (i == 0 || i == h - 1)
        ch = box.horiz;
      else if (j == 0 || j == w - 1)
        ch = box.vert;
      else
        ch = " ";

      size_t len = strlen(ch);
      memcpy(&ascii->buff[i][cursor], ch, len);
      cursor += len;
    }

    ascii->buff[i][cursor] = '\0';
  }

  return ascii;
}

void tui_ascii_free(struct tui_ascii *ascii) {
  for (uint16_t i = 0; i < ascii->h; i++) {
    free(ascii->buff[i]);
  }
  free(ascii->buff);
  free(ascii);
}

void tui_resize(struct tui_ui *ui) {
  struct winsize ws;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

  ui->h = ws.ws_row - 2;
  ui->w = ws.ws_col;
  free(ui->buff);
  free(ui->cells);
  ui->buff = (char *)calloc(ui->h * ui->w * 4, sizeof(char)); // 4 bytes per char
  ui->cells = (struct tui_cell *)malloc(sizeof(struct tui_cell) * ui->h * ui->w);
  for (uint32_t i = 0; i < ui->h * ui->w; i++) {
    ui->cells[i].c[0] = ' ';
    ui->cells[i].size = 1;
  }

  tui_render(ui);
}
