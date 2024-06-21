#ifndef SHOW_NAME_APP
#define SHOW_NAME_APP

#include "app.h"

namespace hitcon {

class ShowNameApp : public App {
public:
  static constexpr int NAME_LEN = 16;
  static constexpr char *DEFAULT_NAME = "HITCON2024";
  char name[NAME_LEN + 1] = {0};

  ShowNameApp();
  virtual ~ShowNameApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void SetName(const char *name);
};

extern ShowNameApp show_name_app;

} // namespace hitcon

#endif // SHOW_NAME_APP
