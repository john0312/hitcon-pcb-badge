#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Logic/EntropyHub.h>
#include <Logic/GameLogic.h>
#include <Logic/PreparedData.h>
#include <Logic/RandomPool.h>
#include <Secret/secret.h>

#include <cstring>

namespace hitcon {

namespace {

using ::hitcon::game::gameLogic;
using ::hitcon::service::sched::scheduler;
using ::hitcon::service::sched::task_callback_t;

}  // namespace

PreparedData g_prepared_data;

PreparedData::PreparedData()
    : current_partition_(0), ir_idx_(-1), xboard_idx_(-1), elapsed_sec_(0),
      partition_change_(-1), init_finish_(false),
      routine_task(985, (task_callback_t)&PreparedData::Routine, this, 1000) {}

void PreparedData::Init() {
  scheduler.Queue(&routine_task, nullptr);
  scheduler.EnablePeriodic(&routine_task);
}

void PreparedData::GetRandomDataForIrTransmission(uint8_t* out_data,
                                                  int* out_col) {
  bool use_prepared = false;
  if (init_finish_ && kIrPartitionSize &&
      (g_fast_random_pool.GetRandom() & 0x0FFFF) < kIrUsePreparedChance) {
    use_prepared = true;
  }
  if (use_prepared) {
    int idx = current_partition_ * kIrPartitionSize + ir_idx_;
    memcpy(out_data, &kStrayIRData[idx * 9 + 1], hitcon::game::kDataSize);
    *out_col = static_cast<int>(kStrayIRData[idx * 9 + 0]);
  } else {
    gameLogic.GetRandomDataForIrTransmission(out_data, out_col);
  }
}

void PreparedData::GetRandomDataForXBoardTransmission(uint8_t* out_data,
                                                      int* out_col) {
  bool use_prepared = false;
  if (init_finish_ && kXBoardPartitionSize &&
      (g_fast_random_pool.GetRandom() & 0x0FFFF) < kXBoardUsePreparedChance) {
    use_prepared = true;
  }
  if (use_prepared) {
    int selected_idx =
        xboard_idx_ + (g_fast_random_pool.GetRandom() % kXBoardWindowSize);
    selected_idx = selected_idx % kXBoardPartitionSize;
    int idx = current_partition_ * kXBoardPartitionSize + selected_idx;
    memcpy(out_data, &kStrayXBoardData[idx * 9 + 1], hitcon::game::kDataSize);
    *out_col = static_cast<int>(kStrayXBoardData[idx * 9 + 0]);
  } else {
    gameLogic.GetRandomDataForIrTransmission(out_data, out_col);
  }
}

void PreparedData::SetPartitionAndShow(int partition) {
  partition_change_ = (partition + kPartitionOffset) & 0x0FF;
}

void PreparedData::Routine() {
  elapsed_sec_++;

  if (!init_finish_ && g_entropy_hub.EntropyReady()) {
    if (kIrPartitionSize) {
      ir_idx_ = g_fast_random_pool.GetRandom() % kIrPartitionSize;
    }
    if (kXBoardPartitionSize) {
      xboard_idx_ = g_fast_random_pool.GetRandom() % kXBoardPartitionSize;
    }
    init_finish_ = true;
  }

  if (init_finish_ && elapsed_sec_ % kPerDataPeriod == 0) {
    if (kIrPartitionSize) {
      ir_idx_ = (ir_idx_ + 1) % kIrPartitionSize;
    }
    if (kXBoardPartitionSize) {
      xboard_idx_ = (xboard_idx_ + 1) % kXBoardPartitionSize;
    }
  }

  if (partition_change_ != -1) {
    SetPartition(partition_change_);
    if (current_partition_ == partition_change_) {
      char msg[5];
      msg[0] = 'P';
      msg[1] = '0' + current_partition_;
      msg[2] = '\0';
      show_name_app.SetSurpriseMsg(msg);
      show_name_app.SetMode(Surprise);
      badge_controller.change_app(&show_name_app);
    }
    partition_change_ = -1;
  }
}

}  // namespace hitcon
