#ifndef LOGIC_PREPARED_DATA_H_
#define LOGIC_PREPARED_DATA_H_

#include <Secret/secret.h>
#include <Service/Sched/Scheduler.h>

namespace hitcon {

class PreparedData {
 public:
  PreparedData();

  void Init();

  void SetPartition(int partition) {
    if (partition >= 0 && partition < kPartitionCount) {
      current_partition_ = partition;
    }
  }

  int GetPartition(int partition) { return current_partition_; }

  void GetRandomDataForIrTransmission(uint8_t* out_data, int* out_col);
  void GetRandomDataForXBoardTransmission(uint8_t* out_data, int* out_col);

 private:
  int current_partition_;
  int ir_idx_;
  int xboard_idx_;

  int elapsed_sec_;

  bool init_finish_;

  hitcon::service::sched::PeriodicTask routine_task;

  void Routine();
};

extern PreparedData g_prepared_data;

}  // namespace hitcon

#endif  // LOGIC_PREPARED_DATA_H_
