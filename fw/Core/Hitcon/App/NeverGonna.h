#ifndef NEVER_GONNA_H
#define NEVER_GONNA_H

#include <App/app.h>

namespace hitcon {
namespace never {

class Gonna : public App {
 public:
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  void GiveYouUp();
  void LetYouDown();
  void RunAroundAndDesertYou();

 private:
  char understand[19] = {};
};

extern Gonna gonna;

}  // namespace never
}  // namespace hitcon

#endif  // NEVER_GONNA_H