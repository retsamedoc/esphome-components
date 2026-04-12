/**
 * @file roomba_ascii.cpp
 * @brief Non-OI CRLF text when the Roomba UART is not in OI stream framing.
 */

#include "roomba_ascii.h"

#include <cctype>
#include <cstdio>
#include <cstring>

#include "esphome/core/log.h"

namespace esphome::roomba {

static const char *TAG = "roomba";

namespace {

const char *ascii_ltrim(const char *s) {
  while (*s != '\0' && std::isspace(static_cast<unsigned char>(*s)))
    s++;
  return s;
}

bool ascii_is_microcontroller_token(const char *s) {
  if (s == nullptr || *s == '\0')
    return false;
  size_t n = std::strlen(s);
  if (n > 16)
    return false;
  bool has_letter = false;
  bool has_digit = false;
  for (size_t i = 0; i < n; i++) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    if (!std::isalnum(c))
      return false;
    if (std::isalpha(c))
      has_letter = true;
    if (std::isdigit(c))
      has_digit = true;
  }
  return has_letter && has_digit;
}

void handle_ascii_line_(const char *line, bool *dock_bat_active) {
  const char *s = ascii_ltrim(line);
  if (*s == '\0')
    return;

  if (std::strncmp(s, "bat:", 4) == 0) {
    unsigned min_v{}, sec_v{}, mv{}, ma{}, tenths{}, mah{}, state_v{}, mode_v{};
    if (std::sscanf(s, "bat: min %u sec %u mV %u mA %u tenths-deg-C %u mAH %u state %u mode %u", &min_v, &sec_v, &mv, &ma,
                    &tenths, &mah, &state_v, &mode_v) == 8) {
      if (dock_bat_active != nullptr)
        *dock_bat_active = true;
      ESP_LOGD(TAG,
               "dock bat: min=%u sec=%u mV=%u mA=%u tenths_deg_c=%u mAH=%u state=%u mode=%u", min_v, sec_v, mv, ma,
               tenths, mah, state_v, mode_v);
    } else {
      ESP_LOGD(TAG, "dock bat (unparsed): %s", s);
    }
    return;
  }

  if (std::strcmp(s, "key-wakeup") == 0) {
    ESP_LOGD(TAG, "button: key-wakeup");
    return;
  }

  if (std::strcmp(s, "bl-start") == 0) {
    ESP_LOGD(TAG, "boot: bl-start");
    return;
  }
  if (std::strcmp(s, "processor-sleep") == 0) {
    ESP_LOGD(TAG, "sleep: processor-sleep");
    return;
  }

  if (std::strncmp(s, "Roomba by iRobot!", 17) == 0) {
    ESP_LOGD(TAG, "boot: %s", s);
    return;
  }

  if (std::strncmp(s, "slept for ", 10) == 0) {
    unsigned smin = 0, ssec = 0;
    if (std::sscanf(s, "slept for %u minutes %u seconds", &smin, &ssec) == 2) {
      ESP_LOGD(TAG, "sleep: slept for %u min %u sec", smin, ssec);
    } else {
      ESP_LOGD(TAG, "sleep (unparsed): %s", s);
    }
    return;
  }

  if (std::strncmp(s, "battery-current-zero", 20) == 0) {
    unsigned z = 0;
    if (std::sscanf(s, "battery-current-zero %u", &z) == 1) {
      ESP_LOGD(TAG, "diag: battery-current-zero=%u", z);
    } else {
      ESP_LOGD(TAG, "diag (unparsed): %s", s);
    }
    return;
  }

  if (std::strstr(s, "r3_robot/") != nullptr || std::strncmp(s, "release-stm32-", 14) == 0 ||
      std::strncmp(s, "release-", 8) == 0) {
    ESP_LOGD(TAG, "firmware_release: %s", s);
    return;
  }

  {
    unsigned y = 0, mo = 0, d = 0, hhmm = 0;
    int consumed = 0;
    if (std::sscanf(s, "%u-%u-%u-%u-L%n", &y, &mo, &d, &hhmm, &consumed) == 4 && s[consumed] == '\0') {
      ESP_LOGD(TAG, "firmware_build: %s", s);
      return;
    }
  }

  if (ascii_is_microcontroller_token(s)) {
    ESP_LOGD(TAG, "microcontroller: %s", s);
    return;
  }

  if (std::strncmp(s, "bootloader id:", 14) == 0) {
    ESP_LOGD(TAG, "bootloader id:%s", s + 14);
    return;
  }
  if (std::strncmp(s, "bootloader info rev:", 20) == 0) {
    ESP_LOGD(TAG, "bootloader info rev:%s", s + 20);
    return;
  }
  if (std::strncmp(s, "bootloader rev:", 15) == 0) {
    ESP_LOGD(TAG, "bootloader rev:%s", s + 15);
    return;
  }

  const char *colon = std::strchr(s, ':');
  if (colon != nullptr && colon != s) {
    ESP_LOGD(TAG, "diag: %s", s);
    return;
  }

  ESP_LOGV(TAG, "ascii: %s", s);
}

}  // namespace

void roomba_ascii_discard_buffer(RoombaAsciiLineBuffer *buf) {
  if (buf == nullptr)
    return;
  buf->fill = 0;
  buf->data[0] = '\0';
}

void roomba_ascii_reset_state(RoombaAsciiLineBuffer *buf, bool *dock_bat_active) {
  roomba_ascii_discard_buffer(buf);
  if (dock_bat_active != nullptr)
    *dock_bat_active = false;
}

void roomba_ascii_feed_wait_header_byte(RoombaAsciiLineBuffer *buf, uint8_t b, bool *dock_bat_active) {
  if (buf == nullptr)
    return;
  if (b == '\r') {
    return;
  }
  if (b == '\n') {
    buf->data[buf->fill] = '\0';
    if (buf->fill > 0)
      handle_ascii_line_(buf->data.data(), dock_bat_active);
    buf->fill = 0;
    return;
  }
  if (b == '\t' || (b >= 0x20 && b <= 0x7E)) {
    if (buf->fill >= buf->data.size() - 1) {
      ESP_LOGW(TAG, "ASCII line overflow; discarding partial line");
      buf->fill = 0;
    } else {
      buf->data[buf->fill++] = static_cast<char>(b);
    }
  }
}

}  // namespace esphome::roomba
