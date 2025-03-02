#include "TetrisApp.h"

#include <App/MainMenuApp.h>
#include <App/ShowNameApp.h>
#include <App/ShowScoreApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameScore.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>

using hitcon::service::sched::SysTimer;
using hitcon::service::sched::task_callback_t;
using namespace hitcon::service::xboard;

namespace hitcon {

namespace app {

namespace tetris {

namespace {

unsigned int tetris_random() { return g_fast_random_pool.GetRandom(); }

}  // namespace

TetrisApp tetris_app;

TetrisApp::TetrisApp()
    : periodic_task(hitcon::tetris::UPDATE_PRIORITY,
                    (task_callback_t)&TetrisApp::periodic_task_callback, this,
                    hitcon::tetris::UPDATE_INTERVAL) {
  hitcon::service::sched::scheduler.Queue(&periodic_task, nullptr);
}

static void SendAttackEnemyPacket(int n_lines) {
  uint8_t data[2] = {PACKET_ATTACK, (uint8_t)n_lines};
  g_xboard_logic.QueueDataForTx(&data[0], 2, TETRIS_RECV_ID);
}

void TetrisApp::OnEntry() {
  // start a new game
  game = hitcon::tetris::TetrisGame(tetris_random);
  display_set_mode_scroll_text("Ready?");
  if (multiplayer) {
    game.game_register_attack_enemy_callback(SendAttackEnemyPacket);
  }

  // start the update task
  hitcon::service::sched::scheduler.EnablePeriodic(&periodic_task);
  g_xboard_logic.SetOnPacketArrive((callback_t)&TetrisApp::OnXboardRecv, this,
                                   TETRIS_RECV_ID);
}

void SetSingleplayer() { tetris_app.SetPlayerCount(SINGLEPLAYER); }

void SetMultiplayer() { tetris_app.SetPlayerCount(MULTIPLAYER); }

void TetrisApp::SetPlayerCount(unsigned playerCount) {
  multiplayer = (playerCount == MULTIPLAYER);
}

void TetrisApp::OnExit() {
  hitcon::service::sched::scheduler.DisablePeriodic(&periodic_task);
}

void TetrisApp::RecvAttackPacket(PacketCallbackArg *packet) {
  if (packet->len != 2) return;
  int n_lines = packet->data[1];
  game.game_enemy_attack(n_lines);
}

void TetrisApp::OnXboardRecv(void *arg) {
  PacketCallbackArg *packet = reinterpret_cast<PacketCallbackArg *>(arg);
  switch (packet->data[0]) {
    case PACKET_GAME_START:
      game.game_start_playing();
      break;

    case PACKET_ATTACK:
      RecvAttackPacket(packet);
      break;

    case PACKET_GAME_OVER:
      game.game_force_over();

      show_score_app.SetScore(game.game_get_score());
      g_game_score.MarkScore(GameScoreType::GAME_TETRIS, game.game_get_score());
      badge_controller.change_app(&show_score_app);
      break;

    case PACKET_ABORT_GAME:
      badge_controller.BackToMenu(this);
      break;
  }
}

void TetrisApp::OnButton(button_t button) {
  switch (game.game_get_state()) {
    case hitcon::tetris::GAME_STATE_WAITING: {
      switch (button) {
        case BUTTON_OK:
          if (multiplayer) {
            uint8_t code = PACKET_GAME_START;
            g_xboard_logic.QueueDataForTx(&code, 1, TETRIS_RECV_ID);
          }
          game.game_start_playing();
          break;
        case BUTTON_BACK:
        case BUTTON_LONG_BACK:
          if (multiplayer) {
            uint8_t code = PACKET_ABORT_GAME;
            g_xboard_logic.QueueDataForTx(&code, 1, TETRIS_RECV_ID);
          }
          badge_controller.BackToMenu(this);
          break;
        default:
          break;
      }
      break;
    }

    case hitcon::tetris::GAME_STATE_GAME_OVER: {
      // after ShowScoreApp is implemented, game over won't be handled here
      break;
    }

    case hitcon::tetris::GAME_STATE_PLAYING: {
      /**
       * Note that we need to rotate the badge by 90 degrees clockwise to play
       * the game. Therefore, the button is remapped.
       */
      switch (button) {
        case BUTTON_LEFT:
          game.game_on_input(hitcon::tetris::DIRECTION_UP);
          break;

        case BUTTON_RIGHT:
          game.game_on_input(hitcon::tetris::DIRECTION_DOWN);
          break;

        case BUTTON_DOWN:
          game.game_on_input(hitcon::tetris::DIRECTION_LEFT);
          break;

        case BUTTON_UP:
          game.game_on_input(hitcon::tetris::DIRECTION_RIGHT);
          break;

        case BUTTON_OK:
          game.game_on_input(hitcon::tetris::DIRECTION_FAST_DOWN);
          break;

        case BUTTON_BACK:
        case BUTTON_LONG_BACK:
          if (multiplayer) {
            uint8_t code = PACKET_ABORT_GAME;
            g_xboard_logic.QueueDataForTx(&code, 1, TETRIS_RECV_ID);
          }
          badge_controller.BackToMenu(this);
          break;

        default:
          break;
      }
    }
  }
}

void TetrisApp::periodic_task_callback(void *) {
  switch (game.game_get_state()) {
    case hitcon::tetris::GAME_STATE_WAITING: {
      break;
    }

    case hitcon::tetris::GAME_STATE_GAME_OVER: {
      if (multiplayer) {
        uint8_t code = PACKET_GAME_OVER;
        g_xboard_logic.QueueDataForTx(&code, 1, TETRIS_RECV_ID);
      }

      show_score_app.SetScore(game.game_get_score());
      g_game_score.MarkScore(GameScoreType::GAME_TETRIS, game.game_get_score());
      badge_controller.change_app(&show_score_app);
      break;
    }

    case hitcon::tetris::GAME_STATE_PLAYING: {
      static int last_fall_time = 0;
      int now = static_cast<int>(SysTimer::GetTime());
      if (game.game_fall_down_if_its_time(now, last_fall_time)) {
        last_fall_time = now;
      }

      // update display buffer
      display_buf_t display_buf[DISPLAY_WIDTH];
      game.game_draw_to_display(display_buf);
      display_set_mode_fixed_packed(display_buf);
      break;
    }
  }
}

}  // namespace tetris

}  // namespace app

}  // namespace hitcon
