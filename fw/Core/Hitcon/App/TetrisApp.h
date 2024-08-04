#ifndef TETRIS_APP_H
#define TETRIS_APP_H

#include <Logic/ButtonLogic.h>
#include <Service/Sched/Scheduler.h>

#include "TetrisGame.h"
#include "app.h"

namespace hitcon {

/**
 * The Tetris game.
 * This app will create a periodic task to update the game state.
 * When ever a button is pressed, it is handled immediately.
 */
class TetrisApp : public App {
 private:
  hitcon::tetris::TetrisGame game;
  hitcon::service::sched::PeriodicTask periodic_task;

  unsigned last_fall_time = 0;

 public:
  TetrisApp();
  virtual ~TetrisApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void periodic_task_callback(void*);
};

extern TetrisApp tetris_app;

}  // namespace hitcon

#endif  // TETRIS_APP_H
