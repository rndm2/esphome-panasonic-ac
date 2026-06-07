#pragma once

#include <cstddef>
#include <cstdint>

namespace esphome {
namespace panasonic_ac {
namespace WLAN {

/*
 * Panasonic PACi UART protocol for the CZ-RTC/CN-CNT controller bus.
 *
 * Scope:
 * - PACi indoor/outdoor systems only.
 * - Developed from captures of S-100PF1E5A indoor + U-100PEY1E5 outdoor.
 * - UART: 2400 baud, 8 data bits, even parity, 1 stop bit.
 * - Normal checksum: XOR of all frame bytes, including checksum, equals 0x00.
 *
 * Addressing rule:
 * - ESP-originated frames must use source 0xE0.
 * - 0x40 belongs to the wired wall controller and is RX/passive only.
 */

static constexpr uint8_t FRAME_SRC_PRIMARY = 0xE0;
static constexpr uint8_t FRAME_SRC_WIRED = static_cast<uint8_t>(0x20 << 1);
static constexpr uint8_t FRAME_SRC_EVENT = 0x00;
// Extended unit/outdoor identity responses can arrive as 00 C0 1A ...
// This source is RX-only. ESP-originated TX remains forced to E0.
static constexpr uint8_t FRAME_SRC_EXTENDED_UNIT = 0xC0;
static constexpr uint8_t FRAME_ADDR_ZERO = 0x00;

static constexpr uint8_t OP_WRITE = 0x11;
static constexpr uint8_t OP_READ = 0x15;
static constexpr uint8_t OP_READ_EXT = 0x17;
static constexpr uint8_t OP_RESPONSE = 0x18;
static constexpr uint8_t OP_RESPONSE_EXT = 0x1A;
static constexpr uint8_t OP_READ_ALT = 0x55;

static constexpr uint8_t RESP_ACK_0 = 0x80;
static constexpr uint8_t RESP_ACK_1 = 0xA1;

static constexpr uint8_t GROUP_CONTROL = 0x08;

static constexpr uint8_t CMD_ADMIN_SETTINGS = 0x07;
static constexpr uint8_t CMD_INFO_MODEL = 0x08;
static constexpr uint8_t CMD_INFO_SERIAL = 0x0B;
static constexpr uint8_t CMD_STATUS_MISC = 0x0C;
static constexpr uint8_t CMD_POWER = 0x41;
static constexpr uint8_t CMD_MODE = 0x42;
static constexpr uint8_t CMD_TEMP_FAN_GROUP = 0x4C;
static constexpr uint8_t CMD_ECO = 0x54;
static constexpr uint8_t CMD_CAPABILITY = 0x58;

static constexpr uint8_t STATUS_MAIN = 0x81;
static constexpr uint8_t STATUS_EXTENDED_UNIT = 0xEF;
static constexpr uint8_t STATUS_EVENT_GROUP_0 = 0x80;
static constexpr uint8_t STATUS_EVENT_GROUP_1 = 0x86;

/* Power */
static constexpr uint8_t POWER_OFF = 0x82;
static constexpr uint8_t POWER_ON = 0x83;

static const uint8_t PAYLOAD_POWER_OFF[]{GROUP_CONTROL, CMD_POWER, POWER_OFF};
static const uint8_t PAYLOAD_POWER_ON[]{GROUP_CONTROL, CMD_POWER, POWER_ON};

/* Modes */
static constexpr uint8_t MODE_HEAT = 0x01;
static constexpr uint8_t MODE_COOL = 0x02;
static constexpr uint8_t MODE_FAN_ONLY = 0x03;
static constexpr uint8_t MODE_DRY = 0x04;
static constexpr uint8_t MODE_AUTO = 0x05;

/* PACi target/current temperature encoding.
 * TEMP_RAW_OFFSET is defined once in panasonic_ac.h and reused here.
 */
static constexpr uint8_t TEMP_SLOT_HEAT = 0x09;
static constexpr uint8_t TEMP_SLOT_COOL = 0x0A;
static constexpr uint8_t TEMP_SLOT_DRY = 0x0C;
static constexpr uint8_t TEMP_SLOT_AUTO_0D = 0x0D;
static constexpr uint8_t TEMP_SLOT_AUTO_0E = 0x0E;
static constexpr uint8_t TEMP_SLOT_AUTO_15 = 0x15;

static constexpr uint8_t TEMP_KIND_AUTO = 0x1D;
static constexpr uint8_t TEMP_KIND_HEAT = 0x2A;
static constexpr uint8_t TEMP_KIND_COOL = 0x1A;
static constexpr uint8_t TEMP_KIND_DRY = 0x1A;

/* Fan slots from PACi wired-controller captures:
 * - HEAT:     08 4C 11 <fan> 00 00
 * - COOL:     08 4C 12 <fan> <target>
 * - FAN_ONLY: 08 4C 13 <fan> 00 00
 * - DRY:      08 4C 14 <fan> 00 00
 * AUTO uses slot 0x15 on the target PACi unit.
 */
static constexpr uint8_t FAN_SLOT_AUTO = 0x15;
static constexpr uint8_t FAN_SLOT_HEAT = 0x11;
static constexpr uint8_t FAN_SLOT_COOL = 0x12;
static constexpr uint8_t FAN_SLOT_FAN_ONLY = 0x13;
static constexpr uint8_t FAN_SLOT_DRY = 0x14;

static constexpr uint8_t FAN_AUTO = 0x1A;
static constexpr uint8_t FAN_HIGH = 0x1B;
static constexpr uint8_t FAN_MEDIUM = 0x1C;
static constexpr uint8_t FAN_LOW = 0x1D;

/* Eco / powersave */
static constexpr uint8_t ECO_OFF = 0x09;
static constexpr uint8_t ECO_ON = 0x0B;

static const uint8_t PAYLOAD_ECO_OFF[]{GROUP_CONTROL, CMD_ECO, ECO_OFF};
static const uint8_t PAYLOAD_ECO_ON[]{GROUP_CONTROL, CMD_ECO, ECO_ON};

/* Compact status/event */
static constexpr uint8_t EVENT_PREFIX_1 = 0x52;
static constexpr uint8_t EVENT_POWER_KEY = 0x84;
static constexpr uint8_t EVENT_POWER_OFF = 0x00;
static constexpr uint8_t EVENT_POWER_ON = 0x01;

/* Main climate status payload after 80 81:
 * S0 S1 S2 S3 TT CT X1 X2 PS
 */
static constexpr uint8_t STATUS_PAYLOAD_POWER_MODE_INDEX = 0;
static constexpr uint8_t STATUS_PAYLOAD_FAN_INDEX = 1;
static constexpr uint8_t STATUS_PAYLOAD_TARGET_TEMP_INDEX = 4;
static constexpr uint8_t STATUS_PAYLOAD_CURRENT_TEMP_CANDIDATE_INDEX = 5;
static constexpr uint8_t STATUS_PAYLOAD_POWERSAVE_CANDIDATE_INDEX = 8;

static constexpr uint8_t STATUS_POWER_MASK = 0x01;
static constexpr uint8_t STATUS_MODE_SHIFT = 5;
static constexpr uint8_t STATUS_MODE_MASK = 0x07;
static constexpr uint8_t STATUS_FAN_SHIFT = 5;
static constexpr uint8_t STATUS_FAN_MASK = 0x07;

static constexpr uint8_t STATUS_MODE_HEAT = 0x01;
static constexpr uint8_t STATUS_MODE_COOL = 0x02;
static constexpr uint8_t STATUS_MODE_FAN_ONLY = 0x03;
static constexpr uint8_t STATUS_MODE_DRY = 0x04;
static constexpr uint8_t STATUS_MODE_AUTO = 0x05;
static constexpr uint8_t STATUS_MODE_AUTO_ALT = 0x06;

static constexpr uint8_t STATUS_FAN_AUTO = 0x02;
static constexpr uint8_t STATUS_FAN_HIGH = 0x03;
static constexpr uint8_t STATUS_FAN_MEDIUM = 0x04;
static constexpr uint8_t STATUS_FAN_LOW = 0x05;

/* Admin / service settings: 08 07 KK VV */
static constexpr uint8_t ADMIN_CODE_VENTILATION_OUTPUT = 0x31;
static constexpr uint8_t ADMIN_CODE_ROOM_TEMP_SENSOR = 0x32;
static constexpr uint8_t ADMIN_CODE_TEMP_DISPLAY_UNIT = 0x33;

static constexpr uint8_t ADMIN_VENTILATION_NOT_CONNECTED = 0x00;
static constexpr uint8_t ADMIN_VENTILATION_CONNECTED = 0x01;

static constexpr uint8_t ADMIN_ROOM_TEMP_SENSOR_MAIN_UNIT = 0x00;
static constexpr uint8_t ADMIN_ROOM_TEMP_SENSOR_REMOTE_CONTROLLER = 0x01;

static constexpr uint8_t ADMIN_TEMP_UNIT_CELSIUS = 0x00;
static constexpr uint8_t ADMIN_TEMP_UNIT_FAHRENHEIT = 0x01;

/* Identity / information */
static constexpr uint8_t INFO_MODEL = CMD_INFO_MODEL;
static constexpr uint8_t INFO_SERIAL = CMD_INFO_SERIAL;
static constexpr uint8_t INFO_UNIT_EXTENDED = STATUS_EXTENDED_UNIT;

}  // namespace WLAN
}  // namespace panasonic_ac
}  // namespace esphome
