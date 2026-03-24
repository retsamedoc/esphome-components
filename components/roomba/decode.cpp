/**
 * @file decode.cpp
 * @brief Packet decoding implementations
 */

#include <cstddef>
#include <cstdint>

#include "roomba.h"
#include "protocol.h"

namespace esphome::roomba {

/* ================= HELPERS ================= */

static constexpr uint16_t output_key(uint8_t p, uint8_t i) { return (static_cast<uint16_t>(p) << 8) | i; }

void publish_u8(Roomba *s, uint8_t p, uint8_t i, float v) {
  Roomba::OutputSlot *slot = s->find_output_slot_(output_key(p, i));
  if (slot && slot->sensor)
    slot->sensor->publish_state(v);
}

/** Publish signed/derived numeric values to a registered numeric sensor slot. */
void publish_s16(Roomba *s, uint8_t p, uint8_t i, int16_t v) {
  Roomba::OutputSlot *slot = s->find_output_slot_(output_key(p, i));
  if (slot && slot->sensor)
    slot->sensor->publish_state(v);
}

/** Publish boolean values to a registered binary sensor slot. */
void publish_bool(Roomba *s, uint8_t p, uint8_t i, bool v) {
  Roomba::OutputSlot *slot = s->find_output_slot_(output_key(p, i));
  if (slot && slot->binary)
    slot->binary->publish_state(v);
}

/** Publish enum/text values to a registered text sensor slot. */
void publish_text(Roomba *s, uint8_t p, uint8_t i, const char *v) {
  Roomba::OutputSlot *slot = s->find_output_slot_(output_key(p, i));
  if (slot && slot->text)
    slot->text->publish_state(v);
}

static inline float mm_to_m(int16_t v) { return v / 1000.0f; }

static inline float mv_to_v(uint16_t v) { return v / 1000.0f; }

template<size_t N> constexpr const char *state_to_str(uint8_t v, const StateTextMap (&table)[N]) {
  for (const auto &entry : table) {
    if (entry.enum_id == v)
      return entry.name;
  }
  return "Unknown";
}

namespace {

/** Single-byte sensor passthrough (packet id -> publish_u8 index 0). */
template<uint8_t PacketId> void decode_pass_u8(Roomba *s, const uint8_t *d) {
  publish_u8(s, PacketId, 0, static_cast<float>(d[0]));
}

}  // namespace

/* ================= PACKET DECODERS ================= */

/* Packet 7 - Bump and Wheel Drop Sensors */
void decode_7(Roomba *s, const uint8_t *d) {
  publish_bool(s, 7, 0, d[0] & 0x01);
  publish_bool(s, 7, 1, d[0] & 0x02);
  publish_bool(s, 7, 2, d[0] & 0x04);
  publish_bool(s, 7, 3, d[0] & 0x08);
}

/* Packet 18 buttons */
void decode_18(Roomba *s, const uint8_t *d) {
  for (int i = 0; i < 8; i++)
    publish_bool(s, 18, i, d[0] & (1 << i));
}

/* Motion */
// Distance (mm → m)
void decode_19(Roomba *s, const uint8_t *d) {
  int16_t mm = (d[0] << 8) | d[1];
  publish_u8(s, 19, 0, mm_to_m(mm));
}

// Angle (degrees)
void decode_20(Roomba *s, const uint8_t *d) {
  int16_t deg = (d[0] << 8) | d[1];
  publish_s16(s, 20, 0, deg);
}

/* Battery */
// Charging state
void decode_21(Roomba *s, const uint8_t *d) { publish_text(s, 21, 0, state_to_str(d[0], ChargingStates)); }

// Voltage (mV → V)
void decode_22(Roomba *s, const uint8_t *d) {
  uint16_t mv = (d[0] << 8) | d[1];
  publish_u8(s, 22, 0, mv_to_v(mv));
}

// Current (mA)
void decode_23(Roomba *s, const uint8_t *d) {
  int16_t ma = (d[0] << 8) | d[1];
  publish_s16(s, 23, 0, ma);
}

// Temperature (°C)
void decode_24(Roomba *s, const uint8_t *d) { publish_u8(s, 24, 0, static_cast<float>(static_cast<int8_t>(d[0]))); }

// Battery charge (mAh, raw)
void decode_25(Roomba *s, const uint8_t *d) { s->charge_ = (static_cast<uint16_t>(d[0]) << 8) | d[1]; }

// Battery %
void decode_26(Roomba *s, const uint8_t *d) {
  s->capacity_ = (d[0] << 8) | d[1];
  if (s->capacity_ > 0) {
    float pct = (float) s->charge_ / s->capacity_ * 100.0f;
    publish_u8(s, 26, 0, pct);
  }
}

// OI Mode
void decode_35(Roomba *s, const uint8_t *d) { publish_text(s, 35, 0, state_to_str(d[0], OIModes)); }

/* Everything else: single-byte passthrough decoders */
void decode_8(Roomba *s, const uint8_t *d) { decode_pass_u8<8>(s, d); }
void decode_9(Roomba *s, const uint8_t *d) { decode_pass_u8<9>(s, d); }
void decode_10(Roomba *s, const uint8_t *d) { decode_pass_u8<10>(s, d); }
void decode_11(Roomba *s, const uint8_t *d) { decode_pass_u8<11>(s, d); }
void decode_12(Roomba *s, const uint8_t *d) { decode_pass_u8<12>(s, d); }
void decode_13(Roomba *s, const uint8_t *d) { decode_pass_u8<13>(s, d); }
void decode_14(Roomba *s, const uint8_t *d) { decode_pass_u8<14>(s, d); }
void decode_15(Roomba *s, const uint8_t *d) { decode_pass_u8<15>(s, d); }
void decode_16(Roomba *s, const uint8_t *d) { decode_pass_u8<16>(s, d); }
void decode_17(Roomba *s, const uint8_t *d) { decode_pass_u8<17>(s, d); }
void decode_43(Roomba *s, const uint8_t *d) { decode_pass_u8<43>(s, d); }
void decode_44(Roomba *s, const uint8_t *d) { decode_pass_u8<44>(s, d); }
void decode_45(Roomba *s, const uint8_t *d) { decode_pass_u8<45>(s, d); }
void decode_46(Roomba *s, const uint8_t *d) { decode_pass_u8<46>(s, d); }
void decode_47(Roomba *s, const uint8_t *d) { decode_pass_u8<47>(s, d); }
void decode_48(Roomba *s, const uint8_t *d) { decode_pass_u8<48>(s, d); }
void decode_49(Roomba *s, const uint8_t *d) { decode_pass_u8<49>(s, d); }
void decode_50(Roomba *s, const uint8_t *d) { decode_pass_u8<50>(s, d); }
void decode_51(Roomba *s, const uint8_t *d) { decode_pass_u8<51>(s, d); }
void decode_52(Roomba *s, const uint8_t *d) { decode_pass_u8<52>(s, d); }
void decode_53(Roomba *s, const uint8_t *d) { decode_pass_u8<53>(s, d); }
void decode_54(Roomba *s, const uint8_t *d) { decode_pass_u8<54>(s, d); }
void decode_55(Roomba *s, const uint8_t *d) { decode_pass_u8<55>(s, d); }
void decode_56(Roomba *s, const uint8_t *d) { decode_pass_u8<56>(s, d); }
void decode_57(Roomba *s, const uint8_t *d) { decode_pass_u8<57>(s, d); }
void decode_58(Roomba *s, const uint8_t *d) { decode_pass_u8<58>(s, d); }

}  // namespace esphome::roomba
