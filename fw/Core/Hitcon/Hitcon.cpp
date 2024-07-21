/*
 * Hitcon.cpp
 *
 *  Created on: Jun 30, 2024
 *      Author: aoaaceai
 */

#include <Hitcon.h>
#include <Logic/IrLogic.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <stdint.h>

using namespace hitcon;
using namespace hitcon::service::sched;

uint8_t hidata[]{2, 3, 5, 7, 11, 13};
size_t len = 6;
void cb(void* unused1, void* unused2) {
  hitcon::ir::irLogic.SendPacket(hidata, len);
}

Task InitTask(1, (task_callback_t)&cb, nullptr);

void test(void* unused1, void* unused2) {}

void hitcon_run() {
  hitcon::ir::irService.Init();
  hitcon::ir::irLogic.Init();
  scheduler.Queue(&InitTask, nullptr);
  scheduler.Run();
}
