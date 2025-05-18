/*
 * Hitcon.cpp
 *
 *  Created on: Jun 30, 2024
 *      Author: aoaaceai
 */

#include <App/DinoApp.h>
#include <App/HardwareTestApp.h>
#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/tama_src/TamaApp.h>
#include <Hitcon.h>
#include <Logic/BadgeController.h>
#include <Logic/ButtonLogic.h>
#include <Logic/DisplayLogic.h>
#include <Logic/EcLogic.h>
#include <Logic/EntropyHub.h>
#include <Logic/GameController.h>
#include <Logic/GameScore.h>
#include <Logic/IrController.h>
#include <Logic/IrLogic.h>
#include <Logic/NvStorage.h>
#include <Logic/UsbLogic.h>
#include <Logic/XBoardLogic.h>
#include <Service/ButtonService.h>
#include <Service/DisplayService.h>
#include <Service/FlashService.h>
#include <Service/HashService.h>
#include <Service/IrService.h>
#include <Service/NoiseSource.h>
#include <Service/Sched/Scheduler.h>
#include <Service/SignedPacketService.h>
#include <Service/XBoardService.h>

using namespace hitcon;
using namespace hitcon::ecc;
using namespace hitcon::hash;
using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;
using namespace hitcon::app::tama;

void TestTaskFunc(void* unused1, void* unused2) {}
void TestTask2Func(void* unused1, void* unused2) {}

Task TestTask1(900, (task_callback_t)&TestTaskFunc, nullptr);
PeriodicTask TestTask2(950, (task_callback_t)&TestTask2Func, nullptr, 201);

void PostSchedInit(void* unused1, void* unused2) {
  // For any initialization that should happen after scheduler is running.
  scheduler.Queue(&TestTask1, nullptr);
  scheduler.Queue(&TestTask2, nullptr);
  scheduler.EnablePeriodic(&TestTask2);
  g_display_service.Init();
  g_display_service.SetBrightness(g_display_brightness);
}

Task InitTask(200, (task_callback_t)&PostSchedInit, nullptr);

void hitcon_run() {
  display_init();
  g_noise_source.Init();
  g_entropy_hub.Init();
  g_hash_service.Init();
  g_fast_random_pool.Init();
  g_secure_random_pool.Init();
  g_ec_logic.Init();
  g_signed_packet_service.Init();
  g_game_controller.Init();
  g_game_score.Init();
  g_flash_service.Init();
  g_nv_storage.Init();
  g_display_logic.Init();

  g_button_logic.Init();
  g_button_service.Init();
  g_xboard_service.Init();
  g_xboard_logic.Init();
  show_name_app.Init();

  // this call shownameapp onentry
  badge_controller.Init();
  hitcon::ir::irService.Init();
  hitcon::ir::irLogic.Init();
  hitcon::ir::irController.Init();
  hitcon::app::snake::snake_app.Init();
  hitcon::app::dino::dino_app.Init();
  hitcon::app::tama::tama_app.Init();
  hitcon::usb::g_usb_logic.Init();

  // run hardware test mode if MODE/SETTINGS Button is pressed during
  // initializing
  if (HAL_GPIO_ReadPin(BtnA_GPIO_Port, BtnA_Pin) == GPIO_PIN_RESET) {
    // Test app needs to be the last to be initialized because otherwise
    // irController may override its callbacks.
    hardware_test_app.Init();
    badge_controller.change_app(&hardware_test_app);
  }

  tama_app.Init();
  badge_controller.change_app(&tama_app);

  scheduler.Queue(&InitTask, nullptr);

  scheduler.Run();
}
