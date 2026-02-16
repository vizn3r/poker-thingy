#include "tui.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

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
  free(ui->buff);
  free(ui->cells);
  free(ui);
  printf("\033[H\033[J");
  printf("\033[?1049l");
  printf("\033[?25h");
  fflush(stdout);
}

void tui_render(struct tui_ui *ui) {
  printf("\033[?25l\033[1;1H");
  fflush(stdout);
  size_t cursor = 0;
  memset(ui->buff, 0, ui->h * ui->w * 4);

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
  for (uint32_t i = 0; i < ui->h * ui->w; i++) {
    ui->cells[i].c[0] = ' ';
    ui->cells[i].size = 1;
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
  if (y >= ui->h)
    return;
  for (uint16_t row = 0; row < ascii->h; row++) {
    uint16_t i = 0;
    uint16_t col = 0;
    while (ascii->buff[row][i] != '\0') {
      if (x + col >= ui->w)
        return;
      size_t size = utf8_char_size(ascii->buff[row][i]);
      memcpy(ui->cells[(y + row) * ui->w + x + col - ascii->w / 2].c, &ascii->buff[row][i], size);
      ui->cells[(y + row) * ui->w + x + col - ascii->w / 2].size = size;
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
}
