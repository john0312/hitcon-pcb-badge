#ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
#define HITCON_SERVICE_BUTTON_SERVICE_H_

#include <cstddef>

#include <Util/callback.h>

namespace hitcon {

class ButtonService {
 public:
  ButtonService();

  void Init();

  // Everytime a collection of kDatasetSize of PA register is collected,
  // this callback will be called with a pointer to an uint16_t array of
  // size kDatasetSize.
  void SetDataInCallback(callback_t callback, void *callback_arg1);
  
  static constexpr size_t kDatasetSize = 4;
};

} // namespace hitcon

#endif // #ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
