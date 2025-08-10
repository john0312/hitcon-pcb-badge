#ifndef SNAKE_APP_H
#define SNAKE_APP_H

#include <App/MultiplayerGame.h>
#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

#include "app.h"

using namespace hitcon::service::sched;

namespace hitcon {
namespace app {
namespace snake {

enum direction_t {
  NONE = 0,
  DIRECTION_RIGHT,
  DIRECTION_LEFT,
  DIRECTION_UP,
  DIRECTION_DOWN,
};

enum {  // XBOARD
  PACKET_GET_FOOD = 1,
  PACKET_GAME_START,
  PACKET_GAME_OVER,
  PACKET_GAME_LEAVE,
};

enum {
  MODE_NONE = 0,
  // wait for game start (player press ok button to start)
  STATE_WAIT,
  STATE_PLAYING,
  STATE_END,
};

// set mode before change to snake app
void SetSingleplayer();
void SetMultiplayer();

class SnakeApp : public hitcon::app::multiplayer::MultiplayerGame {
 private:
  // interval for snake moving
  static constexpr unsigned INTERVAL = 350;

  PeriodicTask _routine_task;
  uint8_t _body[DISPLAY_HEIGHT * DISPLAY_WIDTH];  // 0: head
  uint8_t _len;
  uint8_t _state;
  direction_t _direction;
  direction_t _last_direction;
  int8_t _food_index;
  bool _game_over;
  uint32_t _score;

  void GenerateFood();
  void Routine(void* unused);
  bool OnSnake(uint8_t index);

 protected:
  virtual void GameEntry() override final;
  virtual void GameExit() override final;
  virtual void StartGame() override final;
  virtual void AbortGame() override final;
  virtual void GameOver() override final;
  virtual hitcon::service::xboard::RecvFnId GetXboardRecvId()
      const override final;
  virtual hitcon::game::EventType GetGameType() const override final;
  virtual uint32_t GetScore() const override final;
  virtual void RecvAttackPacket(
      hitcon::service::xboard::PacketCallbackArg* packet) override final;

 public:
  unsigned mode;
  SnakeApp();
  virtual ~SnakeApp() = default;

  void Init();
  // summon random snake and food
  void InitGame();
  void OnButton(button_t button) override;
  void OnEdgeButton(button_t button) override;
};

extern SnakeApp snake_app;

}  // namespace snake
}  // namespace app
}  // namespace hitcon

#endif  // SNAKE_APP_H
