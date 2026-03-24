#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "esphome.h"

#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"

#ifndef ROOMBA_MAX_OUTPUTS
#define ROOMBA_MAX_OUTPUTS 32
#endif

namespace esphome::roomba {

/** Max stream payload bytes (Roomba length field is one byte). */
static constexpr size_t ROOMBA_STREAM_PAYLOAD_MAX = 256;
/** Max single sensor query response (largest packet in PACKETS is well under 64). */
static constexpr size_t ROOMBA_QUERY_BUF_MAX = 64;
/** Max dynamic packet request list (configured outputs + dependency slack). */
static constexpr size_t ROOMBA_MAX_REQUEST_PACKETS = ROOMBA_MAX_OUTPUTS + 8;

/**
 * @brief ESPHome component for Roomba Open Interface communication.
 *
 * This class owns UART transport, stream frame parsing, packet decode dispatch,
 * output registration, and reconnect/reset behavior.
 *
 * @note The runtime is stream-first with query-mode fallback for non-stream use.
 */
class Roomba : public Component, public uart::UARTDevice {
 public:
  /** Initialize optional GPIOs and perform initial Roomba recovery sequence. */
  void setup() override;
  /** Process UART bytes, parser state, and recovery timers. */
  void loop() override;
  /** Print component configuration and runtime flags. */
  void dump_config() override;
  /** Transition to safe shutdown by sending STOP then SAFE. */
  void on_shutdown() override;

  /** Enable or disable OI stream mode. */
  void set_use_stream(bool v) { this->use_stream_ = v; }
  /** Enable or disable silence-driven reconnect attempts. */
  void set_auto_reconnect(bool v) { this->auto_reconnect_ = v; }
  /** Re-issue the last command after recovery when possible. */
  void set_restore_state(bool v) { this->restore_state_ = v; }
  /** Provide optional BRC pin used for wake/reset pulses. */
  void set_brc_pin(GPIOPin *pin) { this->brc_pin_ = pin; }
  /** Provide optional time source used for clock sync. */
  void set_time_source(time::RealTimeClock *time_source) { this->time_source_ = time_source; }

  /**
   * @brief Register a numeric sensor destination.
   * @param packet Roomba packet id.
   * @param index Output index inside decoded packet.
   * @param s Sensor entity pointer.
   */
  void register_sensor(uint8_t packet, uint8_t index, sensor::Sensor *s);
  /**
   * @brief Register a binary sensor destination.
   * @param packet Roomba packet id.
   * @param index Output index inside decoded packet.
   * @param b Binary sensor entity pointer.
   */
  void register_binary(uint8_t packet, uint8_t index, binary_sensor::BinarySensor *b);
  /**
   * @brief Register a text sensor destination.
   * @param packet Roomba packet id.
   * @param index Output index inside decoded packet.
   * @param t Text sensor entity pointer.
   */
  void register_text(uint8_t packet, uint8_t index, text_sensor::TextSensor *t);

  /** Send CLEAN command and persist command intent for optional restore. */
  void start_clean();
  /** Send SEEK_DOCK command and persist command intent for optional restore. */
  void dock();
  /** Send STOP command and clear command intent. */
  void stop();
  /** Send RESET command. */
  void reset();
  /** Send SET_DAY_TIME (168) using the current valid time source. */
  void set_day_time();

 protected:
  friend void publish_u8(Roomba *, uint8_t, uint8_t, float);
  friend void publish_s16(Roomba *, uint8_t, uint8_t, int16_t);
  friend void publish_bool(Roomba *, uint8_t, uint8_t, bool);
  friend void publish_text(Roomba *, uint8_t, uint8_t, const char *);
  friend void decode_25(Roomba *, const uint8_t *);
  friend void decode_26(Roomba *, const uint8_t *);

  /** Send one opcode with internal command-rate limiting. */
  void send_opcode(uint8_t op);

  /** Start OI stream subscription for the default packet list. */
  void start_stream_();
  /** Parse one UART byte through stream framing state machine. */
  void parse_byte_(uint8_t b);
  /** Process a fully validated stream payload buffer. */
  void process_stream_();
  /** Decode a packet payload by id using PACKETS table callbacks. */
  void decode_packet_(uint8_t id, const uint8_t *data);
  /** Return expected payload size for packet id from PACKETS table. */
  uint8_t get_packet_size_(uint8_t id);
  /** Return true when packet id exists in protocol packet table. */
  bool is_known_packet_id_(uint8_t id) const;
  /** Add packet id to request list if absent and capacity allows. */
  void append_request_packet_(uint8_t id);
  /** Add packet id and required decoder dependencies. */
  void append_packet_with_dependencies_(uint8_t id);
  /** Rebuild dynamic request packet list from registered outputs. */
  void rebuild_requested_packets_();

  /** Emit a SENSORS query command for a single packet id. */
  void query_sensors_(uint8_t id);
  /** Consume one byte from a pending SENSORS query response. */
  void parse_query_byte_(uint8_t b);
  /** Clear pending query state and buffered response bytes. */
  void reset_query_state_();

  /** Wake the robot using a short BRC pulse when configured. */
  void wake_roomba_();
  /** Long BRC pulse (same timing as hard_reset_) without logging; no-op if no BRC pin. */
  void long_brc_pulse_reset_();
  /**
   * @brief Reinitialize parser state and OI session.
   * @param is_boot True when called from initial component setup.
   */
  void recover_roomba_(bool is_boot);
  /** Perform a longer BRC pulse reset sequence when configured. */
  void hard_reset_();
  /** Validate time object before using it for Roomba day/time sync. */
  bool is_time_valid_(const ESPTime &now) const;
  /** Write SET_DAY_TIME with payload derived from the provided time. */
  bool set_day_time_from_now_(const ESPTime &now);

  /**
   * @brief Output slot for any entity type
   */
  struct OutputSlot {
    sensor::Sensor *sensor{nullptr};
    binary_sensor::BinarySensor *binary{nullptr};
    text_sensor::TextSensor *text{nullptr};
  };

  /**
   * @brief Linear lookup for (packet<<8)|index.
   * @param key Packed packet/index key.
   * @return Slot pointer when found, otherwise nullptr.
   */
  OutputSlot *find_output_slot_(uint16_t key);

  // Config
  bool use_stream_{true};
  bool auto_reconnect_{true};
  bool restore_state_{false};
  bool shutting_down_{false};

  GPIOPin *brc_pin_{nullptr};
  time::RealTimeClock *time_source_{nullptr};
  bool initial_time_sync_done_{false};

  // State machine

  /**
   * @brief Roomba stream parsing state machine
   */
  enum class ParseState {
    WAIT_HEADER,   ///< Waiting for packet header
    WAIT_LENGTH,   ///< Waiting for packet length byte
    READ_PAYLOAD,  ///< Reading packet payload
    READ_CHECKSUM  ///< Reading checksum byte
  };

  ParseState state_{ParseState::WAIT_HEADER};
  std::array<uint8_t, ROOMBA_STREAM_PAYLOAD_MAX> stream_payload_{};
  size_t payload_fill_{0};
  uint8_t length_{0};    ///< Expected payload length from stream
  uint8_t checksum_{0};  ///< Running checksum

  // Query state
  uint8_t query_expected_{0};
  uint8_t query_id_{0};
  std::array<uint8_t, ROOMBA_QUERY_BUF_MAX> query_buf_{};
  size_t query_fill_{0};
  bool query_pending_{false};
  uint32_t query_started_at_{0};
  uint8_t query_poll_index_{0};

  // Outputs

  struct OutputEntry {
    uint16_t key{0};
    OutputSlot slot{};
  };

  std::array<OutputEntry, ROOMBA_MAX_OUTPUTS> outputs_{};
  size_t outputs_count_{0};
  std::array<uint8_t, ROOMBA_MAX_REQUEST_PACKETS> requested_packets_{};
  size_t requested_packets_count_{0};

  // Battery tracking
  uint16_t charge_{0};
  uint16_t capacity_{0};

  // Recovery
  uint32_t last_data_{0};
  uint32_t last_init_{0};
  uint32_t last_recovery_at_{0};
  uint32_t last_cmd_time_{0};
  uint8_t recovery_attempts_{0};
  bool stream_frame_seen_{false};
  uint32_t stream_checksum_errors_{0};
  uint32_t last_stream_no_rx_warn_{0};

  enum LastCommand { CMD_NONE, CMD_CLEAN, CMD_DOCK };

  LastCommand last_cmd_{CMD_NONE};
};

template<typename... Ts> class SetDayTimeAction : public Action<Ts...> {
 public:
  explicit SetDayTimeAction(Roomba *parent) : parent_(parent) {}

  void play(const Ts &...) override { this->parent_->set_day_time(); }

 protected:
  Roomba *parent_;
};

template<typename... Ts> class StartCleanAction : public Action<Ts...> {
 public:
  explicit StartCleanAction(Roomba *parent) : parent_(parent) {}

  void play(const Ts &...) override { this->parent_->start_clean(); }

 protected:
  Roomba *parent_;
};

template<typename... Ts> class DockAction : public Action<Ts...> {
 public:
  explicit DockAction(Roomba *parent) : parent_(parent) {}

  void play(const Ts &...) override { this->parent_->dock(); }

 protected:
  Roomba *parent_;
};

template<typename... Ts> class StopAction : public Action<Ts...> {
 public:
  explicit StopAction(Roomba *parent) : parent_(parent) {}

  void play(const Ts &...) override { this->parent_->stop(); }

 protected:
  Roomba *parent_;
};

template<typename... Ts> class ResetAction : public Action<Ts...> {
 public:
  explicit ResetAction(Roomba *parent) : parent_(parent) {}

  void play(const Ts &...) override { this->parent_->reset(); }

 protected:
  Roomba *parent_;
};

}  // namespace esphome::roomba
