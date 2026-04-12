#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome::roomba {

/** Max CRLF-terminated non-OI ASCII line (dock `bat:`, boot diagnostics). */
static constexpr size_t ROOMBA_ASCII_LINE_MAX = 192;

/**
 * @brief Buffer for one incoming ASCII line while waiting for OI stream header 0x13.
 */
struct RoombaAsciiLineBuffer {
  std::array<char, ROOMBA_ASCII_LINE_MAX> data{};
  size_t fill{0};
};

void roomba_ascii_discard_buffer(RoombaAsciiLineBuffer *buf);
void roomba_ascii_reset_state(RoombaAsciiLineBuffer *buf, bool *dock_bat_active);
void roomba_ascii_feed_wait_header_byte(RoombaAsciiLineBuffer *buf, uint8_t b, bool *dock_bat_active);

}  // namespace esphome::roomba
