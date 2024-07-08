#ifdef HITCON_TEST_MODE

#include <stdio.h>
#include <unistd.h>

#include "game.h"

#define STORAGE_PATH "/tmp/pcg-game.dat"

void read_persistent_storage(storage_t *storage) {
  FILE *file = fopen(STORAGE_PATH, "rb");
  if (file) {
    fread(storage, sizeof(storage_t), 1, file);
    fclose(file);
  } else {
    storage = NULL;
  }
}

void write_persistent_storage(storage_t *storage) {
  FILE *file = fopen(STORAGE_PATH, "wb");
  fwrite(storage, sizeof(storage_t), 1, file);
  fclose(file);
}

int main() {
  // load previous game state if exists
  storage_t storage;
  read_persistent_storage(&storage);
  game_init(&storage);

  // register_callbacks();

  // main loop
  int time = 0;
  for (;; ++time, sleep(1)) {
    // TODO: call periodic functions

    // TODO: simulate events

    // save game state
    storage = game_get_storage();
    write_persistent_storage(&storage);
  }

  return 0;
}

#endif
