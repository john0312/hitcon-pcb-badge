/*
 * Hitcon.cpp
 *
 *  Created on: Jun 30, 2024
 *      Author: aoaaceai
 */

#include <App/HardwareTestApp.h>
#include <Hitcon.h>
#include <Logic/BadgeController.h>
#include <Logic/ButtonLogic.h>
#include <Logic/DisplayLogic.h>
#include <Logic/IrLogic.h>
#include <Logic/NvStorage.h>
#include <Logic/XBoardLogic.h>
#include <Logic/IrController.h>
#include <Service/ButtonService.h>
#include <Service/DisplayService.h>
#include <Service/FlashService.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <Service/XBoardService.h>
#include <stdint.h>

using namespace hitcon;
using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;

void TestTaskFunc(void* unused1, void* unused2) {}

Task TestTask1(900, (task_callback_t)&TestTaskFunc, nullptr);

void PostSchedInit(void* unused1, void* unused2) {
  // For any initialization that should happen after scheduler is running.
  scheduler.Queue(&TestTask1, nullptr);
}

Task InitTask(200, (task_callback_t)&PostSchedInit, nullptr);

void hitcon_run() {
  display_init();
  g_flash_service.Init();
  g_nv_storage.Init();
  g_display_logic.Init();
  g_display_service.Init();
  g_display_service.SetBrightness(3);
  g_button_logic.Init();
  g_button_service.Init();
  g_xboard_service.Init();
  g_xboard_logic.Init();
  badge_controller.Init();
  hitcon::ir::irService.Init();
  hitcon::ir::irLogic.Init();

  // run hardware test mode if MODE/SETTINGS Button is pressed during
  // initializing
  if (HAL_GPIO_ReadPin(BtnA_GPIO_Port, BtnA_Pin) == GPIO_PIN_RESET) {
    hardware_test_app.Init();
    badge_controller.change_app(&hardware_test_app);
  }

  hitcon::ir::irService.Init();
  hitcon::ir::irLogic.Init();
  hitcon::ir::irController.Init();
  scheduler.Queue(&InitTask, nullptr);

  scheduler.Run();
}
