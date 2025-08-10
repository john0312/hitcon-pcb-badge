#ifndef TETRIS_APP_H
#define TETRIS_APP_H

#include <App/MultiplayerGame.h>
#include <Logic/ButtonLogic.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/Scheduler.h>

#include "TetrisGame.h"
#include "app.h"

namespace hitcon {

namespace app {

namespace tetris {

// set mode before changing to tetris app
void SetSingleplayer();
void SetMultiplayer();

/**
 * The first byte of the packet is the packet tpe
 * 1. attack packet:
 *    Bytes 2 is the number of lines attacked
 */

/**
 * The Tetris game.
 * This app will create a periodic task to update the game state.
 * When ever a button is pressed, it is handled immediately.
 */
class TetrisApp : public hitcon::app::multiplayer::MultiplayerGame {
 private:
  hitcon::tetris::TetrisGame game;
  hitcon::service::sched::PeriodicTask periodic_task;

 protected:
  virtual void GameEntry() override final;
  virtual void GameExit() override final;
  virtual void StartGame() override final;
  virtual void AbortGame() override final;
  virtual void GameOver() override final;
  virtual hitcon::service::xboard::RecvFnId GetXboardRecvId()
      const override final;
  virtual hitcon::game::EventType GetGameType() const override final;
  virtual void RecvAttackPacket(
      hitcon::service::xboard::PacketCallbackArg *packet) override final;
  virtual uint32_t GetScore() const override final;

 public:
  TetrisApp();
  virtual ~TetrisApp() = default;

  void OnButton(button_t button) override final;

  void periodic_task_callback(void *);
};

extern TetrisApp tetris_app;

}  // namespace tetris

}  // namespace app

}  // namespace hitcon

#endif  // TETRIS_APP_H
