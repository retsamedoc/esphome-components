/**
 * @file roomba.cpp
 * @brief Runtime implementation for the Roomba ESPHome component.
 */
#include "roomba.h"
#include "protocol.h"
#include "roomba_ascii.h"

#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/macros.h"

namespace esphome::roomba {

static const char *TAG = "roomba";
static constexpr uint32_t QUERY_TIMEOUT_MS = 500;

/* ================= SETUP ================= */

void Roomba::setup() {
  if (this->brc_pin_) {
    this->brc_pin_->setup();
    this->brc_pin_->digital_write(true);
  }

  delay(1000);
}

void Roomba::dump_config() {
  ESP_LOGCONFIG(TAG, "Roomba:");
  ESP_LOGCONFIG(TAG, "  Stream: %s", YESNO(this->use_stream_));
  ESP_LOGCONFIG(TAG, "  Auto reconnect: %s", YESNO(this->auto_reconnect_));
  ESP_LOGCONFIG(TAG, "  Restore state: %s", YESNO(this->restore_state_));
  ESP_LOGCONFIG(TAG, "  Time source: %s", this->time_source_ != nullptr ? "configured" : "none");
}

void Roomba::on_shutdown() {
  this->shutting_down_ = true;
  this->send_opcode(static_cast<uint8_t>(Opcode::STOP));
  delay(100);
  this->send_opcode(static_cast<uint8_t>(Opcode::SAFE));
}

/* ================= LOOP ================= */

void Roomba::loop() {
  if (this->shutting_down_)
    return;

  if (this->use_stream_ && this->query_pending_) {
    this->reset_query_state_();
  }

  uint32_t now = millis();
  if (!this->initial_time_sync_done_ && this->time_source_ != nullptr) {
    this->set_day_time();
    if (this->initial_time_sync_done_) {
      ESP_LOGI(TAG, "Roomba clock synced from initial valid time");
    }
  }

  if (this->query_pending_ && (now - this->query_started_at_ > QUERY_TIMEOUT_MS)) {
    ESP_LOGW(TAG, "Timed out waiting for query response (packet %u)", this->query_id_);
    this->reset_query_state_();
  }

  while (this->available()) {
    uint8_t b;
    this->read_byte(&b);
    if (this->query_pending_ && (!this->use_stream_ || (this->state_ == ParseState::WAIT_HEADER && b != 19))) {
      this->parse_query_byte_(b);
    } else {
      this->parse_byte_(b);
    }
  }

  uint32_t silence = now - this->last_data_;

  static constexpr uint32_t RECONNECT_GRACE_MS = 5000;

  if (this->auto_reconnect_) {
    if (silence < 1000)
      this->recovery_attempts_ = 0;

    if (now - this->last_init_ >= RECONNECT_GRACE_MS) {
      if (silence > 3000 && this->recovery_attempts_ == 0) {
        this->recover_roomba_(false);
        this->recovery_attempts_ = 1;
      } else if (silence > 7000 && this->recovery_attempts_ == 1) {
        this->wake_roomba_();
        this->recover_roomba_(false);
        this->recovery_attempts_ = 2;
      } else if (silence > 15000 && this->recovery_attempts_ >= 2) {
        this->hard_reset_();
        this->recover_roomba_(false);
        this->recovery_attempts_ = 0;
      }
    }
  }

  if (!this->use_stream_) {
    static uint32_t last_poll = 0;
    if (!this->query_pending_ && now - last_poll > 1000) {
      if (this->requested_packets_count_ == 0) {
        ESP_LOGW(TAG, "No configured packets for query mode; polling packet %u fallback",
                 static_cast<uint8_t>(Packet::BUMPS_WHEELDROPS));
        this->query_sensors_(static_cast<uint8_t>(Packet::BUMPS_WHEELDROPS));
      } else {
        const size_t idx = this->query_poll_index_ % this->requested_packets_count_;
        this->query_sensors_(this->requested_packets_[idx]);
        this->query_poll_index_ = static_cast<uint8_t>((this->query_poll_index_ + 1) % this->requested_packets_count_);
      }
      last_poll = now;
    }
  }
}

/* ================= COMMANDS ================= */

/**
 * @brief Write an opcode to UART with a fixed minimum spacing.
 *
 * The command spacing protects the robot from bursty command sends during
 * recovery or Home Assistant command bursts.
 */
void Roomba::send_opcode(uint8_t op) {
  if (millis() - this->last_cmd_time_ < 100)
    return;
  this->last_cmd_time_ = millis();
  this->write_byte(op);
}

void Roomba::start_clean() {
  this->send_opcode(static_cast<uint8_t>(Opcode::CLEAN));
  this->last_cmd_ = CMD_CLEAN;
}

void Roomba::dock() {
  this->send_opcode(static_cast<uint8_t>(Opcode::SEEK_DOCK));
  this->last_cmd_ = CMD_DOCK;
}

void Roomba::stop() {
  this->send_opcode(static_cast<uint8_t>(Opcode::STOP));
  this->last_cmd_ = CMD_NONE;
}

void Roomba::reset() { this->send_opcode(static_cast<uint8_t>(Opcode::RESET)); }

void Roomba::set_day_time() {
  if (this->time_source_ == nullptr) {
    ESP_LOGW(TAG, "Cannot set day/time: no time source configured");
    return;
  }
  const ESPTime now = this->time_source_->now();
  if (!this->set_day_time_from_now_(now)) {
    return;
  }
  this->initial_time_sync_done_ = true;
}

/* ================= RECOVERY ================= */

/** @brief Keep the robot awake through a short BRC pulse when the pin is configured. */
void Roomba::nudge_roomba_() {
  if (!this->brc_pin_)
    return;

  ESP_LOGI(TAG, "Nudge Roomba");

  this->brc_pin_->digital_write(false);
  delay(100);
  this->brc_pin_->digital_write(true);
  delay(2000);
}

/** @brief Wake the robot through BRC pulse when the pin is configured. */
void Roomba::wake_roomba_() {
  if (!this->brc_pin_)
    return;

  ESP_LOGI(TAG, "Wake Roomba");

  this->brc_pin_->digital_write(false);
  delay(500);
  this->brc_pin_->digital_write(true);
  delay(2000);
}

/** @brief Perform a long reset pulse on BRC pin when the pin is configured. */
void Roomba::hard_reset_() {
  if (!this->brc_pin_)
    return;

  ESP_LOGI(TAG, "Hard reset");

  this->brc_pin_->digital_write(false);
  delay(1000);
  this->brc_pin_->digital_write(true);
  delay(2000);
}

bool Roomba::is_time_valid_(const ESPTime &now) const { return now.is_valid(); }

bool Roomba::set_day_time_from_now_(const ESPTime &now) {
  if (!this->is_time_valid_(now)) {
    ESP_LOGW(TAG, "Cannot set day/time: time source not valid yet");
    return false;
  }
  if (millis() - this->last_cmd_time_ < 100) {
    return false;
  }

  uint8_t roomba_day = 0;
  if (now.day_of_week >= 1 && now.day_of_week <= 7) {
    roomba_day = static_cast<uint8_t>(now.day_of_week - 1);  // Roomba uses Sunday=0..Saturday=6.
  } else {
    roomba_day = static_cast<uint8_t>(now.day_of_week % 7);
  }
  const uint8_t roomba_hour = static_cast<uint8_t>(now.hour);
  const uint8_t roomba_minute = static_cast<uint8_t>(now.minute);

  this->last_cmd_time_ = millis();
  this->write_byte(static_cast<uint8_t>(Opcode::SET_DAY_TIME));
  this->write_byte(roomba_day);
  this->write_byte(roomba_hour);
  this->write_byte(roomba_minute);
  ESP_LOGI(TAG, "Sent SET_DAY_TIME day=%u hour=%u minute=%u", roomba_day, roomba_hour, roomba_minute);
  return true;
}

/**
 * @brief Reinitialize communication with the robot.
 *
 * Sequence: clear parser state, wake, send START x3, enter SAFE, optionally
 * start stream, then optionally replay last command when restore is enabled.
 */
void Roomba::recover_roomba_(bool is_boot) {
  ESP_LOGI(TAG, "Recovering Roomba");

  this->state_ = ParseState::WAIT_HEADER;
  this->payload_fill_ = 0;
  this->reset_query_state_();
  roomba_ascii_reset_state(&this->ascii_line_, &this->dock_bat_status_active_);

  this->nudge_roomba_();

  for (int i = 0; i < 3; i++) {
    this->send_opcode(static_cast<uint8_t>(Opcode::START));
    delay(100);
  }

  this->send_opcode(static_cast<uint8_t>(Opcode::SAFE));
  delay(100);
  this->last_cmd_time_ = millis();

  if (this->use_stream_) {
    this->start_stream_();
  }

  this->last_data_ = millis();
  this->last_init_ = millis();

  if (this->restore_state_ && !is_boot) {
    if (this->last_cmd_ == CMD_CLEAN)
      this->start_clean();
    else if (this->last_cmd_ == CMD_DOCK)
      this->dock();
  }
}

/* ================= STREAM ================= */

/**
 * @brief Start stream mode for the currently selected default packet set.
 *
 * Packet list is static and intentionally conservative for broad compatibility.
 */
void Roomba::start_stream_() {
  this->rebuild_requested_packets_();
  if (this->requested_packets_count_ == 0) {
    ESP_LOGW(TAG, "No configured packets for stream mode; using fallback packet %u",
             static_cast<uint8_t>(Packet::BUMPS_WHEELDROPS));
    this->requested_packets_[0] = static_cast<uint8_t>(Packet::BUMPS_WHEELDROPS);
    this->requested_packets_count_ = 1;
  }

  ESP_LOGI(TAG, "Requesting %u packet(s) in stream/query list", static_cast<unsigned>(this->requested_packets_count_));
  for (size_t i = 0; i < this->requested_packets_count_; i++) {
    ESP_LOGD(TAG, "  packet[%u]=%u", static_cast<unsigned>(i), this->requested_packets_[i]);
  }

  this->write_byte(static_cast<uint8_t>(Opcode::STREAM));
  this->write_byte(static_cast<uint8_t>(this->requested_packets_count_));

  for (size_t i = 0; i < this->requested_packets_count_; i++) {
    this->write_byte(this->requested_packets_[i]);
  }
  this->last_cmd_time_ = millis();
}

/**
 * @brief Parse one incoming byte from UART
 * @param b Incoming byte
 */
void Roomba::parse_byte_(uint8_t b) {
  this->last_data_ = millis();

  switch (this->state_) {
    case ParseState::WAIT_HEADER:
      if (b == 19) {  // Header byte (start of sensor stream)
        roomba_ascii_discard_buffer(&this->ascii_line_);
        this->payload_fill_ = 0;
        this->checksum_ = b;
        this->state_ = ParseState::WAIT_LENGTH;
      } else {
        roomba_ascii_feed_wait_header_byte(&this->ascii_line_, b, &this->dock_bat_status_active_);
      }
      break;

    case ParseState::WAIT_LENGTH:
      this->length_ = b;
      this->payload_fill_ = 0;
      this->checksum_ += b;
      if (this->length_ > ROOMBA_STREAM_PAYLOAD_MAX) {
        ESP_LOGW(TAG, "Invalid stream length %u", this->length_);
        this->state_ = ParseState::WAIT_HEADER;
        break;
      }
      if (this->length_ == 0) {
        this->state_ = ParseState::READ_CHECKSUM;
      } else {
        this->state_ = ParseState::READ_PAYLOAD;
      }
      break;

    case ParseState::READ_PAYLOAD:
      if (this->payload_fill_ < this->length_) {
        this->stream_payload_[this->payload_fill_++] = b;
        this->checksum_ += b;
      }
      if (this->payload_fill_ >= this->length_) {
        this->state_ = ParseState::READ_CHECKSUM;
      }
      break;

    case ParseState::READ_CHECKSUM:
      this->checksum_ += b;
      if ((this->checksum_ & 0xFF) == 0) {
        // Checksum correct, process the payload
        this->process_stream_();
      } else {
        ESP_LOGW(TAG, "Checksum error for packet");
      }
      // Reset state machine
      this->state_ = ParseState::WAIT_HEADER;
      this->payload_fill_ = 0;
      this->length_ = 0;
      this->checksum_ = 0;
      break;
  }
}

/**
 * @brief Process a complete payload after successful checksum
 */
void Roomba::process_stream_() {
  size_t i = 0;

  while (i < this->payload_fill_) {
    uint8_t id = this->stream_payload_[i++];

    // Lookup packet size from table
    uint8_t size = this->get_packet_size_(id);
    if (i + size > this->payload_fill_) {
      ESP_LOGW(TAG, "Payload truncated for packet %d", id);
      break;
    }

    // Decode the packet using table-driven decoder
    this->decode_packet_(id, &this->stream_payload_[i]);
    i += size;
  }
}

/* ================= DECODE ================ */

uint8_t Roomba::get_packet_size_(uint8_t id) {
  for (size_t i = 0; i < NUM_PACKETS; i++) {
    if (static_cast<uint8_t>(PACKETS[i].id) == id)
      return PACKETS[i].size;
  }
  return 1;  // fallback
}

bool Roomba::is_known_packet_id_(uint8_t id) const {
  for (size_t i = 0; i < NUM_PACKETS; i++) {
    if (PACKETS[i].id == id) {
      return true;
    }
  }
  return false;
}

void Roomba::append_request_packet_(uint8_t id) {
  for (size_t i = 0; i < this->requested_packets_count_; i++) {
    if (this->requested_packets_[i] == id) {
      return;
    }
  }
  if (this->requested_packets_count_ >= this->requested_packets_.size()) {
    ESP_LOGW(TAG, "Requested packet list capacity exceeded; dropping packet %u", id);
    return;
  }
  this->requested_packets_[this->requested_packets_count_++] = id;
}

void Roomba::append_packet_with_dependencies_(uint8_t id) {
  // Packet 26 (capacity) decode publishes battery percent and depends on packet 25 (charge).
  if (id == static_cast<uint8_t>(Packet::CAPACITY)) {
    this->append_request_packet_(static_cast<uint8_t>(Packet::CHARGE));
  }
  this->append_request_packet_(id);
}

void Roomba::rebuild_requested_packets_() {
  this->requested_packets_count_ = 0;
  this->query_poll_index_ = 0;

  for (size_t i = 0; i < this->outputs_count_; i++) {
    const uint8_t id = static_cast<uint8_t>(this->outputs_[i].key >> 8);
    if (!this->is_known_packet_id_(id)) {
      ESP_LOGW(TAG, "Skipping unsupported configured packet id %u", id);
      continue;
    }
    this->append_packet_with_dependencies_(id);
  }
}

/**
 * @brief Decode a single packet payload by id.
 *
 * Lookup is table-driven through PACKETS and can be extended by adding entries
 * in protocol.h plus matching decode functions.
 */
void Roomba::decode_packet_(uint8_t id, const uint8_t *data) {
  for (size_t i = 0; i < NUM_PACKETS; i++) {
    if (static_cast<uint8_t>(PACKETS[i].id) == id) {
      if (PACKETS[i].decode)
        PACKETS[i].decode(this, data);
      return;
    }
  }

  ESP_LOGD(TAG, "Unknown packet %d", id);
}

/* ================= QUERY ================= */

/**
 * @brief Emit a one-shot SENSORS query for packet id.
 */
void Roomba::query_sensors_(uint8_t id) {
  if (this->query_pending_)
    return;

  this->write_byte(static_cast<uint8_t>(Opcode::SENSORS));
  this->write_byte(id);

  this->query_id_ = id;
  this->query_expected_ = this->get_packet_size_(id);
  if (this->query_expected_ == 0 || this->query_expected_ > this->query_buf_.size()) {
    ESP_LOGW(TAG, "Invalid query size %u for packet %u", this->query_expected_, id);
    this->reset_query_state_();
    return;
  }
  this->query_fill_ = 0;
  this->query_pending_ = true;
  this->query_started_at_ = millis();
}

void Roomba::parse_query_byte_(uint8_t b) {
  this->last_data_ = millis();

  if (this->query_fill_ >= this->query_expected_ || this->query_fill_ >= this->query_buf_.size()) {
    ESP_LOGW(TAG, "Query buffer overflow for packet %u", this->query_id_);
    this->reset_query_state_();
    return;
  }

  this->query_buf_[this->query_fill_++] = b;
  if (this->query_fill_ == this->query_expected_) {
    this->decode_packet_(this->query_id_, this->query_buf_.data());
    this->reset_query_state_();
  }
}

void Roomba::reset_query_state_() {
  this->query_id_ = 0;
  this->query_expected_ = 0;
  this->query_fill_ = 0;
  this->query_pending_ = false;
  this->query_started_at_ = 0;
}

/** @brief Find output destination slot for packed (packet,index) key. */
Roomba::OutputSlot *Roomba::find_output_slot_(uint16_t key) {
  for (size_t i = 0; i < this->outputs_count_; i++) {
    if (this->outputs_[i].key == key)
      return &this->outputs_[i].slot;
  }
  return nullptr;
}

/* ================= REGISTER OUTPUTS ================= */

/**
 * @brief Register or replace numeric sensor output for packet/index.
 *
 * Replaces existing entry for the same key and clears other entity pointers.
 */
void Roomba::register_sensor(uint8_t packet, uint8_t index, sensor::Sensor *s) {
  uint16_t key = (static_cast<uint16_t>(packet) << 8) | index;
  for (size_t i = 0; i < this->outputs_count_; i++) {
    if (this->outputs_[i].key == key) {
      this->outputs_[i].slot.sensor = s;
      this->outputs_[i].slot.binary = nullptr;
      this->outputs_[i].slot.text = nullptr;
      this->rebuild_requested_packets_();
      return;
    }
  }
  if (this->outputs_count_ >= this->outputs_.size()) {
    ESP_LOGE(TAG, "ROOMBA_MAX_OUTPUTS exceeded");
    return;
  }
  this->outputs_[this->outputs_count_].key = key;
  this->outputs_[this->outputs_count_].slot = {s, nullptr, nullptr};
  this->outputs_count_++;
  this->rebuild_requested_packets_();
}

/**
 * @brief Register or replace binary sensor output for packet/index.
 *
 * Replaces existing entry for the same key and clears other entity pointers.
 */
void Roomba::register_binary(uint8_t packet, uint8_t index, binary_sensor::BinarySensor *b) {
  uint16_t key = (static_cast<uint16_t>(packet) << 8) | index;
  for (size_t i = 0; i < this->outputs_count_; i++) {
    if (this->outputs_[i].key == key) {
      this->outputs_[i].slot.sensor = nullptr;
      this->outputs_[i].slot.binary = b;
      this->outputs_[i].slot.text = nullptr;
      this->rebuild_requested_packets_();
      return;
    }
  }
  if (this->outputs_count_ >= this->outputs_.size()) {
    ESP_LOGE(TAG, "ROOMBA_MAX_OUTPUTS exceeded");
    return;
  }
  this->outputs_[this->outputs_count_].key = key;
  this->outputs_[this->outputs_count_].slot = {nullptr, b, nullptr};
  this->outputs_count_++;
  this->rebuild_requested_packets_();
}

/**
 * @brief Register or replace text sensor output for packet/index.
 *
 * Replaces existing entry for the same key and clears other entity pointers.
 */
void Roomba::register_text(uint8_t packet, uint8_t index, text_sensor::TextSensor *t) {
  uint16_t key = (static_cast<uint16_t>(packet) << 8) | index;
  for (size_t i = 0; i < this->outputs_count_; i++) {
    if (this->outputs_[i].key == key) {
      this->outputs_[i].slot.sensor = nullptr;
      this->outputs_[i].slot.binary = nullptr;
      this->outputs_[i].slot.text = t;
      this->rebuild_requested_packets_();
      return;
    }
  }
  if (this->outputs_count_ >= this->outputs_.size()) {
    ESP_LOGE(TAG, "ROOMBA_MAX_OUTPUTS exceeded");
    return;
  }
  this->outputs_[this->outputs_count_].key = key;
  this->outputs_[this->outputs_count_].slot = {nullptr, nullptr, t};
  this->outputs_count_++;
  this->rebuild_requested_packets_();
}

}  // namespace esphome::roomba
