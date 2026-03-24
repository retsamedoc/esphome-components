#pragma once
/**
 * @file decode.h
 * @brief Decoder entry points used by the packet definition table.
 */

#include <cstdint>

namespace esphome::roomba {

class Roomba;

/** @name Core packet decoders */
/**@{*/
/** Packet 7: bump and wheel drop bitfield. */
void decode_7(Roomba *, const uint8_t *);
/** Packet 8: wall detect bit. */
void decode_8(Roomba *, const uint8_t *);
/** Packet 9: cliff left bit. */
void decode_9(Roomba *, const uint8_t *);
/** Packet 10: cliff front-left bit. */
void decode_10(Roomba *, const uint8_t *);
/** Packet 11: cliff front-right bit. */
void decode_11(Roomba *, const uint8_t *);
/** Packet 12: cliff right bit. */
void decode_12(Roomba *, const uint8_t *);
/** Packet 13: virtual wall bit. */
void decode_13(Roomba *, const uint8_t *);
/** Packet 14: wheel overcurrent status. */
void decode_14(Roomba *, const uint8_t *);
/** Packet 15: one-byte dirt/status value. */
void decode_15(Roomba *, const uint8_t *);
/** Packet 16: one-byte passthrough value. */
void decode_16(Roomba *, const uint8_t *);
/** Packet 17: one-byte passthrough value. */
void decode_17(Roomba *, const uint8_t *);
/** Packet 18: button bitfield. */
void decode_18(Roomba *, const uint8_t *);
/** Packet 19: signed distance in mm (published as meters). */
void decode_19(Roomba *, const uint8_t *);
/** Packet 20: signed angle in degrees. */
void decode_20(Roomba *, const uint8_t *);
/** Packet 21: charging state enum (published as text). */
void decode_21(Roomba *, const uint8_t *);
/** Packet 22: voltage in mV (published as volts). */
void decode_22(Roomba *, const uint8_t *);
/** Packet 23: signed current value. */
void decode_23(Roomba *, const uint8_t *);
/** Packet 24: signed temperature in Celsius. */
void decode_24(Roomba *, const uint8_t *);
/** Packet 25: battery charge cache in mAh. */
void decode_25(Roomba *, const uint8_t *);
/** Packet 26: battery capacity cache + computed percent publish. */
void decode_26(Roomba *, const uint8_t *);
/** Packet 35: OI mode enum (published as text). */
void decode_35(Roomba *, const uint8_t *);
/**@}*/

/** @name Extended packet decoders */
/**@{*/
void decode_43(Roomba *, const uint8_t *);
void decode_44(Roomba *, const uint8_t *);
void decode_45(Roomba *, const uint8_t *);
void decode_46(Roomba *, const uint8_t *);
void decode_47(Roomba *, const uint8_t *);
void decode_48(Roomba *, const uint8_t *);
void decode_49(Roomba *, const uint8_t *);
void decode_50(Roomba *, const uint8_t *);
void decode_51(Roomba *, const uint8_t *);
void decode_52(Roomba *, const uint8_t *);
void decode_53(Roomba *, const uint8_t *);
void decode_54(Roomba *, const uint8_t *);
void decode_55(Roomba *, const uint8_t *);
void decode_56(Roomba *, const uint8_t *);
void decode_57(Roomba *, const uint8_t *);
void decode_58(Roomba *, const uint8_t *);
/**@}*/

}  // namespace esphome::roomba
