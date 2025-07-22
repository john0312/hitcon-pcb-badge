#include <Service/PerBoardData.h>

namespace hitcon {

PerBoardData g_per_board_data;

namespace {

constexpr uint8_t kPerBoardRandom[PerBoardData::kRandomLen] = {
    0xf1, 0xca, 0x4e, 0xa0, 0x48, 0x2f, 0x27, 0x4d,
    0x3d, 0xc2, 0x9c, 0x8c, 0xec, 0x36, 0x83, 0x49};
constexpr uint8_t kPerBoardSecret[PerBoardData::kSecretLen] = {
    0x13, 0xac, 0x76, 0xfc, 0x1a, 0xa7, 0x0f, 0x92,
    0x05, 0x31, 0x1d, 0xa6, 0x28, 0x4c, 0x8e, 0x94};
constexpr uint8_t kPubKeyCert[PerBoardData::kPubKeyCertSize] = {
    0x7d, 0xf0, 0xde, 0x4c, 0xe2, 0x23, 0x19,
    0xf6, 0xb4, 0xfa, 0xbe, 0x12, 0x6d, 0x41};

constexpr uint64_t kPrivKey = 13447439586820736;
}  // namespace

constexpr PerBoardData::PerBoardData() {}

const uint8_t* PerBoardData::GetPerBoardRandom() { return kPerBoardRandom; }

const uint8_t* PerBoardData::GetPerBoardSecret() { return kPerBoardSecret; }

const uint8_t* PerBoardData::GetPubKeyCert() { return kPubKeyCert; }

const uint64_t PerBoardData::GetPrivKey() { return kPrivKey; }
}  // namespace hitcon
