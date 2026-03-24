#pragma once
/**
 * @file protocol.h
 * @brief Roomba Open Interface protocol definitions (opcodes, packets, table)
 */

#include <stdint.h>
#include <cstddef>
#include "decode.h"

namespace esphome::roomba {

class Roomba;

/* ================= MODELS ================= */

/**
 * @brief Roomba model series list
 */
enum RoombaSeries { ROBOT_UNKNOWN, ROBOT_500, ROBOT_600, ROBOT_700, ROBOT_900 };

/**
 * @brief Definition of a Roomba OI State <-> Text Sensor
 */
struct StateTextMap {
  uint8_t enum_id;   ///< Enumerated values from Roomba OI spec
  const char *name;  ///< Human-Readable State
};

/**
 * @brief Charging state
 */
enum class ChargingState : uint8_t {
  NOT_CHARGING = 0,
  RECONDITIONING = 1,
  FULL_CHARGING = 2,
  TRICKLE_CHARGING = 3,
  WAITING = 4,
  CHARGING_FAULT = 5
};

/**
 * @brief OI Mode <-> Text Map
 */
constexpr StateTextMap ChargingStates[] = {{static_cast<uint8_t>(ChargingState::NOT_CHARGING), "Not Charging"},
                                           {static_cast<uint8_t>(ChargingState::RECONDITIONING), "Reconditioning"},
                                           {static_cast<uint8_t>(ChargingState::FULL_CHARGING), "Full Charging"},
                                           {static_cast<uint8_t>(ChargingState::TRICKLE_CHARGING), "Trickle Charging"},
                                           {static_cast<uint8_t>(ChargingState::WAITING), "Waiting"},
                                           {static_cast<uint8_t>(ChargingState::CHARGING_FAULT), "Charging Fault"}};

/**
 * @brief OI Mode
 */
enum class OIMode : uint8_t {
  OFF = 0,
  PASSIVE = 1,
  SAFE = 2,
  FULL = 3,
};

/**
 * @brief OI Mode <-> Text Map
 */
constexpr StateTextMap OIModes[] = {
    {static_cast<uint8_t>(OIMode::OFF), "Off"},
    {static_cast<uint8_t>(OIMode::PASSIVE), "Passive"},
    {static_cast<uint8_t>(OIMode::SAFE), "Safe"},
    {static_cast<uint8_t>(OIMode::FULL), "Full"},
};

/**
 * @brief Roomba OI opcodes
 */
enum class Opcode : uint8_t {
  RESET = 7,
  START = 128,
  BAUD = 129,
  CONTROL = 130,
  SAFE = 131,
  FULL = 132,
  POWER = 133,
  SPOT = 134,
  CLEAN = 135,
  MAX = 136,
  DRIVE = 137,    //  4 bytes: [Velocity high byte] [Velocity low byte] [Radius
                  //  high byte] [Radius low byte]
  MOTORS = 138,   //  1 byte:  [Motors]
  LEDS = 139,     //  3 bytes: [LED bits] [Power Color] [Power Intensity]
  SONG = 140,     // 2n+2 bytes
  PLAY = 141,     //  1 byte: [Song Number]
  SENSORS = 142,  // [Packet ID]
  SEEK_DOCK = 143,
  PWM_MOTORS = 144,    //  3 bytes:  [Main Brush PWM] [Side Brush PWM] [Vacumm PWM]
  DRIVE_DIRECT = 145,  //  4 bytes: [Right velocity high byte] [Right velocity low byte]
                       //  [Left velocity high byte] [Left velocity low byte]
  DRIVE_PWM = 146,     //  4 bytes: [Right PWM high byte] [Right PWM low byte] [Left
                       //  PWM high byte] [Left PWM low byte]
  QUERY_LIST = 149,    // [Number of Packets][Packet ID 1][Packet ID 2]...[Packet ID N]
  STREAM = 148,        // [Number of packets] [Packet ID 1] [Packet ID 2] [Packet ID 3]
  PAUSE_RESUME = 150,
  SCHEDULING_LEDS = 162,  //  2 bytes:  [Weekday LED Bits][Scheduling LED Bits]
  LEDS_ASCII = 164,       //  4 bytes: [Digit 3 ASCII] [Digit 2 ASCII] [Digit 1 ASCII]
                          //  [Digit 0 ASCII]
  BUTTONS = 165,          //  1 byte:  [Buttons]
  SCHEDULE = 167,         // 15 bytes: [Days] [Sun Hour] [Sun Minute] [Mon Hour] [Mon Minute]
                          // [Tue Hour] [Tue Minute] [Wed Hour] [Wed Minute] [Thu Hour] [Thu
                          // Minute] [Fri Hour] [Fri Minute] [Sat Hour] [Sat Minute]
  SET_DAY_TIME = 168,     //  3 bytes: [Day] [Hour] [Minute]
  STOP = 173
};

/**
 * @brief Definition of a Roomba OI sensor packet
 *
 * Each packet corresponds to a sensor value returned by the Roomba Open
 * Interface.
 *
 * @note Used in the PACKETS constexpr table for decoding and sensor
 * registration.
 */
struct PacketDef {
  uint8_t id;                                 ///< Packet ID used in Roomba OI queries
  const char *name;                           ///< Human-Readable Sensor Name
  uint8_t size;                               ///< Size in bytes
  uint8_t series_mask;                        ///< Bitmask of supported Roomba series
  void (*decode)(Roomba *, const uint8_t *);  ///< Decode function
};

/**
 * @brief Packet IDs (subset shown; extended below)
 */
enum class Packet : uint8_t {
  GROUP0 = 0,  // Sensors  7 - 26
  GROUP1 = 1,  // Sensors  7 - 16
  GROUP2 = 2,  // Sensors 17 - 20
  GROUP3 = 3,  // Sensors 21 - 26
  GROUP4 = 4,  // Sensors 27 - 34
  GROUP5 = 5,  // Sensors 35 - 42
  GROUP6 = 6,
  BUMPS_WHEELDROPS = 7,
  WALL = 8,
  CLIFF_LEFT = 9,
  CLIFF_FRONT_LEFT = 10,
  CLIFF_FRONT_RIGHT = 11,
  CLIFF_RIGHT = 12,
  VIRTUAL_WALL = 13,
  WHEEL_OVERCURRENTS = 14,
  DIRT_DETECT = 15,
  IR_CHAR_OMNI = 17,
  BUTTONS = 18,
  DISTANCE = 19,
  ANGLE = 20,
  CHARGING_STATE = 21,
  VOLTAGE = 22,
  CURRENT = 23,
  TEMPERATURE = 24,
  CHARGE = 25,
  CAPACITY = 26,
  OI_MODE = 35,
  ENC_COUNTS_LEFT = 43,
  ENC_COUNTS_RIGHT = 44,
  LIGHT_BUMPER = 45,
  LIGHT_BUMP_LEFT = 46,
  LIGHT_BUMP_FRONT_LEFT = 47,
  LIGHT_BUMP_CENTER_LEFT = 48,
  LIGHT_BUMP_CENTER_RIGHT = 49,
  LIGHT_BUMP_FRONT_RIGHT = 50,
  LIGHT_BUMP_RIGHT = 51,
  IR_CHAR_LEFT = 52,
  IR_CHAR_RIGHT = 53,
  MOTOR_CURRENT_LEFT = 54,
  MOTOR_CURRENT_RIGHT = 55,
  MAIN_BRUSH_CURRENT = 56,
  SIDE_BRUSH_CURRENT = 57,
  STASIS = 58,
  ALL_SENSORS = 100,
  EXTENDED = 101,
  CURRENTS = 107
};

/**
 * @brief Table of all supported Roomba OI sensor packets
 *
 * This table drives:
 * - Sensor decoding
 * - Documentation generation
 *
 * @note This is the single source of truth for packet id/size/decode mapping.
 * @note Packet IDs should be unique and size must match decoder expectations.
 */
constexpr PacketDef PACKETS[] = {
    // Packet ID (ENUM),                 "friendly name",      size, series_mask, decode function
    //  {Packet::GROUP0, "Group 0 (7–26)", 26,     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_0},
    //  {Packet::GROUP1, "Group 1 (7–16)", 10,     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_1},
    //  {Packet::GROUP2, "Group 2 (17–20)", 6,     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_2},
    //  {Packet::GROUP5, "Group 5 (35–42)", 12, ROBOT_600 | ROBOT_700 | ROBOT_900,     decode_5},
    //  {Packet::GROUP6, "Group 6 (7–42)", 52, ROBOT_600 | ROBOT_700 | ROBOT_900,     decode_6},
    {static_cast<uint8_t>(Packet::BUMPS_WHEELDROPS), "Bump & Wheel Drop", 1,
     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_7},
    {static_cast<uint8_t>(Packet::WALL), "Wall", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_8},
    {static_cast<uint8_t>(Packet::CLIFF_LEFT), "Cliff Left", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_9},
    {static_cast<uint8_t>(Packet::CLIFF_FRONT_LEFT), "Cliff Front Left", 1,
     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_10},
    {static_cast<uint8_t>(Packet::CLIFF_FRONT_RIGHT), "Cliff Front Right", 1,
     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_11},
    {static_cast<uint8_t>(Packet::CLIFF_RIGHT), "Cliff Right", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_12},
    {static_cast<uint8_t>(Packet::VIRTUAL_WALL), "Virtual Wall", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_13},
    {static_cast<uint8_t>(Packet::WHEEL_OVERCURRENTS), "Overcurrents", 1, ROBOT_600 | ROBOT_700 | ROBOT_900, decode_14},
    {static_cast<uint8_t>(Packet::DIRT_DETECT), "Dirt Detect", 1, ROBOT_600 | ROBOT_700 | ROBOT_900, decode_15},
    {static_cast<uint8_t>(Packet::IR_CHAR_OMNI), "Infrared Character Omni", 1,
     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_17},
    {static_cast<uint8_t>(Packet::BUTTONS), "Buttons", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_18},
    {static_cast<uint8_t>(Packet::DISTANCE), "Distance", 2, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_19},
    {static_cast<uint8_t>(Packet::ANGLE), "Angle", 2, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_20},
    {static_cast<uint8_t>(Packet::CHARGING_STATE), "Charging State", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_21},
    {static_cast<uint8_t>(Packet::VOLTAGE), "Voltage", 2, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_22},
    {static_cast<uint8_t>(Packet::CURRENT), "Current", 2, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_23},
    {static_cast<uint8_t>(Packet::TEMPERATURE), "Temperature", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_24},
    {static_cast<uint8_t>(Packet::CHARGE), "Battery Charge", 2, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_25},
    {static_cast<uint8_t>(Packet::CAPACITY), "Battery Capacity", 2, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_26},
    {static_cast<uint8_t>(Packet::OI_MODE), "OI Mode", 1, ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_35},
    {static_cast<uint8_t>(Packet::ENC_COUNTS_LEFT), "Encoder Counts Left", 2, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_43},
    {static_cast<uint8_t>(Packet::ENC_COUNTS_RIGHT), "Encoder Counts Right", 2, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_44},
    {static_cast<uint8_t>(Packet::LIGHT_BUMPER), "Light Bumper", 1, ROBOT_600 | ROBOT_700 | ROBOT_900, decode_45},
    {static_cast<uint8_t>(Packet::LIGHT_BUMP_LEFT), "Light Bump Left", 1, ROBOT_600 | ROBOT_700 | ROBOT_900, decode_46},
    {static_cast<uint8_t>(Packet::LIGHT_BUMP_FRONT_LEFT), "Light Bump Front Left", 1, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_47},
    {static_cast<uint8_t>(Packet::LIGHT_BUMP_CENTER_LEFT), "Light Bump Center Left", 1,
     ROBOT_600 | ROBOT_700 | ROBOT_900, decode_48},
    {static_cast<uint8_t>(Packet::LIGHT_BUMP_CENTER_RIGHT), "Light Bump Center Right", 1,
     ROBOT_600 | ROBOT_700 | ROBOT_900, decode_49},
    {static_cast<uint8_t>(Packet::LIGHT_BUMP_FRONT_RIGHT), "Light Bump Front Right", 1,
     ROBOT_600 | ROBOT_700 | ROBOT_900, decode_50},
    {static_cast<uint8_t>(Packet::LIGHT_BUMP_RIGHT), "Light Bump Right", 1, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_51},
    {static_cast<uint8_t>(Packet::IR_CHAR_LEFT), "Infrared Charater Left", 1, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_52},
    {static_cast<uint8_t>(Packet::IR_CHAR_RIGHT), "Infrared Charater Right", 1, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_53},
    {static_cast<uint8_t>(Packet::MOTOR_CURRENT_LEFT), "Left Motor Current", 2, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_54},
    {static_cast<uint8_t>(Packet::MOTOR_CURRENT_RIGHT), "Right Motor Current", 2, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_55},
    {static_cast<uint8_t>(Packet::MAIN_BRUSH_CURRENT), "Main Brush Current", 2, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_56},
    {static_cast<uint8_t>(Packet::SIDE_BRUSH_CURRENT), "Side Brush Current", 2, ROBOT_600 | ROBOT_700 | ROBOT_900,
     decode_57},
    {static_cast<uint8_t>(Packet::STASIS), "Stasis", 1, ROBOT_600 | ROBOT_700 | ROBOT_900, decode_58},
    //    {Packet::ALL_SENSORS, "All Sensors (7–58)" 80,     ROBOT_500 | ROBOT_600 | ROBOT_700 | ROBOT_900, decode_100},
    //    {Packet::EXTENDED, "Extended (43–58)", 28,     ROBOT_600 | ROBOT_700 | ROBOT_900, decode_101},
    //    {       Packet : CURRENTS,       "Currents (46–51)",    12,      ROBOT_600 | ROBOT_700 | ROBOT_900, decode_106
    //    },
};

/** Number of entries in PACKETS, used by runtime linear lookups. */
constexpr size_t NUM_PACKETS = sizeof(PACKETS) / sizeof(PACKETS[0]);

}  // namespace esphome::roomba
