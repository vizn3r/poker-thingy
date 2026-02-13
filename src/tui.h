#pragma once

#include <stdarg.h>
#include <stdbool.h>
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

// Basic functions

// Initializes a new tui_ui struct
struct tui_ui *tui_init(void);

// Frees a tui_ui struct
void tui_free(struct tui_ui *ui);

// Renders the tui_ui struct to the screen
void tui_render(struct tui_ui *ui);

// Sets the tui_ui struct to the given dimensions
void tui_set(struct tui_ui *ui, uint16_t x, uint16_t y, char c);

// Clears the buffer
void tui_clear(struct tui_ui *ui);

// Prints a string to the buffer
void tui_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text);

// Prints a centered string to the buffer
void tui_centered_text(struct tui_ui *ui, uint16_t x, uint16_t y, char *text);

// ASCII functions

// Prints a formatted string to the buffer
void tui_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii);

// Initializes a new tui_ascii struct
void tui_centered_ascii(struct tui_ui *ui, uint16_t x, uint16_t y, struct tui_ascii *ascii);

// Creates a new tui_ascii struct from an array of strings
struct tui_ascii *tui_ascii_arr(char **arr, size_t len);

// Creates a new tui_ascii box
struct tui_ascii *tui_ascii_box(size_t w, size_t h);

// Checks if the tui should be resized
bool tui_resize(struct tui_ui *ui, uint16_t x, uint16_t y);

char *tui_fmt(char *text, ...);
