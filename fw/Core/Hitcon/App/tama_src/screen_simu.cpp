#define constexpr const
#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 8
typedef unsigned char uint8_t;

#include <iostream>

#include "screens.h"

uint8_t full_screen[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {0};

// TODO: add more material array to use
using hitcon::app::tama::screens::cat;

// TODO: add more stack function for different alignment method or purpose
void stack_target_full(const uint8_t* layer, uint8_t* base) {
  /*
  example of stack const layer onto base

  layer: the layer to be stacked, should be const
  base: the base to be stacked on
  */
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      base[y * DISPLAY_WIDTH + x] =
          layer[y * DISPLAY_WIDTH + x] | base[y * DISPLAY_WIDTH + x];
    }
  }
}

void display_input(uint8_t* buf) {
  /*
  To show the full screen
  buf: the buffer to be displayed
  */
  std::cout << "┌";
  for (int x = 0; x < DISPLAY_WIDTH; ++x) std::cout << "──";
  std::cout << "┐\n";

  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    std::cout << "│";
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      uint8_t pixel = buf[y * DISPLAY_WIDTH + x];
      std::cout << (pixel ? "██" : "  ");
    }
    std::cout << "│\n";
  }

  std::cout << "└";
  for (int x = 0; x < DISPLAY_WIDTH; ++x) std::cout << "──";
  std::cout << "┘\n";
}

int main() {
  // TODO: add more stack target
  // stack target onto blank
  stack_target_full(cat, full_screen);

  // display_input
  display_input(full_screen);

  return 0;
}