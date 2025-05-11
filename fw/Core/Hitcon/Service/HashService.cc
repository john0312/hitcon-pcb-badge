#include <Service/HashService.h>

namespace hitcon {

HashService g_hash_service;

void HashService::Init() {}

bool HashService::StartHash(uint8_t const *message, size_t len,
                            callback_t callback, void *callback_arg1) {
  return false;
}

}  // namespace hitcon