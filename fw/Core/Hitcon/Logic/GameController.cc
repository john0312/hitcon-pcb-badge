#include <Logic/EcLogic.h>
#include <Logic/GameController.h>
#include <Service/PerBoardData.h>

namespace hitcon {

hitcon::game::GameController g_game_controller;

namespace game {

GameController::GameController() {}

void GameController::Init() {
  hitcon::ecc::g_ec_logic.SetPrivateKey(g_per_board_data.GetPrivKey());
}

}  // namespace game
}  // namespace hitcon
