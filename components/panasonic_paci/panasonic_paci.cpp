#include "panasonic_paci.h"

#include <cmath>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace panasonic_paci {

static const char *const TAG = "panasonic_paci";

climate::ClimateTraits PanasonicPaci::traits() {
  auto traits = climate::ClimateTraits();

  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_ACTION | climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);

  traits.set_visual_min_temperature(MIN_TEMPERATURE);
  traits.set_visual_max_temperature(MAX_TEMPERATURE);
  traits.set_visual_temperature_step(TEMPERATURE_STEP);

  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_HEAT_COOL,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_DRY,
  });

  // Standard Home Assistant fan modes. This is what HA sends when you change fan
  // mode from the normal climate card.
  traits.set_supported_fan_modes({
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
  });

  // Home Assistant climate card exposes Eco as a preset, not only as a separate switch.
  traits.set_supported_presets({
      climate::CLIMATE_PRESET_NONE,
      climate::CLIMATE_PRESET_ECO,
  });

  return traits;
}

void PanasonicPaci::setup() {
  this->init_time_ = millis();

  ESP_LOGI(TAG, "Panasonic AC component v%s starting...", VERSION);
}

void PanasonicPaci::loop() { this->read_data(); }

void PanasonicPaci::read_data() {
  while (this->available()) {
    if (this->rx_buffer_.size() >= BUFFER_SIZE) {
      ESP_LOGW(TAG, "RX buffer overflow, clearing %u bytes", this->rx_buffer_.size());
      this->rx_buffer_.clear();
    }

    uint8_t c;
    if (this->read_byte(&c)) {
      this->rx_buffer_.push_back(c);
    }
  }
}

uint8_t PanasonicPaci::encode_temperature_raw(float celsius) {
  return static_cast<uint8_t>(std::round(celsius * 2.0f + TEMP_RAW_OFFSET));
}

float PanasonicPaci::decode_temperature_raw(uint8_t raw) {
  return (static_cast<float>(raw) - TEMP_RAW_OFFSET) / 2.0f;
}

bool PanasonicPaci::is_valid_temperature_(float temperature) const {
  return !std::isnan(temperature) && temperature >= TEMPERATURE_MIN_VALID && temperature <= TEMPERATURE_MAX_VALID;
}

void PanasonicPaci::update_target_temperature_from_raw(uint8_t raw_value) {
  this->update_target_temperature_value(decode_temperature_raw(raw_value));
}

void PanasonicPaci::update_target_temperature_value(float temperature) {
  if (!this->is_valid_temperature_(temperature)) {
    ESP_LOGW(TAG, "Ignoring invalid target temperature: %.2f", temperature);
    return;
  }

  this->target_temperature = temperature;

  if (this->target_temperature_sensor_ != nullptr) {
    this->target_temperature_sensor_->publish_state(temperature);
  }
}

void PanasonicPaci::update_current_temperature_from_raw(uint8_t raw_value) {
  this->update_current_temperature_value(decode_temperature_raw(raw_value));
}

void PanasonicPaci::update_current_temperature_value(float temperature) {
  if (!this->is_valid_temperature_(temperature)) {
    ESP_LOGW(TAG, "Ignoring invalid current temperature: %.2f", temperature);
    return;
  }

  this->current_temperature = temperature;

  if (this->current_temperature_sensor_ != nullptr) {
    this->current_temperature_sensor_->publish_state(temperature);
  }
}


void PanasonicPaci::update_eco(bool eco) {
  this->eco_state_ = eco;
  this->preset = eco ? climate::CLIMATE_PRESET_ECO : climate::CLIMATE_PRESET_NONE;

  if (this->eco_switch_ != nullptr) {
    this->eco_switch_->publish_state(eco);
  }
}


void PanasonicPaci::update_ventilation_output(bool connected) {
  this->ventilation_output_connected_ = connected;
  this->ventilation_output_known_ = true;

  if (this->ventilation_output_select_ != nullptr) {
    this->ventilation_output_select_->publish_state(connected ? "Connected" : "Not Connected");
  }
}

void PanasonicPaci::update_remote_temperature_sensor(bool remote_controller) {
  this->remote_temperature_sensor_ = remote_controller;
  this->remote_temperature_sensor_known_ = true;

  if (this->remote_temperature_sensor_select_ != nullptr) {
    this->remote_temperature_sensor_select_->publish_state(remote_controller ? "Remote Controller" : "Main Unit");
  }
}

void PanasonicPaci::update_fahrenheit(bool fahrenheit) {
  this->fahrenheit_unit_ = fahrenheit;
  this->fahrenheit_unit_known_ = true;

  if (this->temperature_unit_select_ != nullptr) {
    this->temperature_unit_select_->publish_state(fahrenheit ? "Fahrenheit" : "Celsius");
  }
}

void PanasonicPaci::update_indoor_model(const std::string &value) {
  if (this->indoor_model_text_sensor_ != nullptr) {
    this->indoor_model_text_sensor_->publish_state(value);
  }
}

void PanasonicPaci::update_indoor_serial(const std::string &value) {
  if (this->indoor_serial_text_sensor_ != nullptr) {
    this->indoor_serial_text_sensor_->publish_state(value);
  }
}

void PanasonicPaci::update_outdoor_model(const std::string &value) {
  if (this->outdoor_model_text_sensor_ != nullptr) {
    this->outdoor_model_text_sensor_->publish_state(value);
  }
}

void PanasonicPaci::update_outdoor_serial(const std::string &value) {
  if (this->outdoor_serial_text_sensor_ != nullptr) {
    this->outdoor_serial_text_sensor_->publish_state(value);
  }
}



climate::ClimateAction PanasonicPaci::determine_action() {
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    return climate::CLIMATE_ACTION_OFF;
  }

  if (this->mode == climate::CLIMATE_MODE_FAN_ONLY) {
    return climate::CLIMATE_ACTION_FAN;
  }

  if (this->mode == climate::CLIMATE_MODE_DRY) {
    return climate::CLIMATE_ACTION_DRYING;
  }

  if (std::isnan(this->current_temperature) || std::isnan(this->target_temperature)) {
    return climate::CLIMATE_ACTION_IDLE;
  }

  if ((this->mode == climate::CLIMATE_MODE_COOL || this->mode == climate::CLIMATE_MODE_HEAT_COOL) &&
      this->current_temperature + TEMPERATURE_TOLERANCE >= this->target_temperature) {
    return climate::CLIMATE_ACTION_COOLING;
  }

  if ((this->mode == climate::CLIMATE_MODE_HEAT || this->mode == climate::CLIMATE_MODE_HEAT_COOL) &&
      this->current_temperature - TEMPERATURE_TOLERANCE <= this->target_temperature) {
    return climate::CLIMATE_ACTION_HEATING;
  }

  return climate::CLIMATE_ACTION_IDLE;
}

void PanasonicPaci::set_eco_switch(switch_::Switch *eco_switch) {
  this->eco_switch_ = eco_switch;

  this->eco_switch_->add_on_state_callback([this](bool state) {
    if (state == this->eco_state_) {
      return;
    }
    this->on_eco_change(state);
  });
}

void PanasonicPaci::set_ventilation_output_select(PanasonicPaciSelect *ventilation_output_select) {
  this->ventilation_output_select_ = ventilation_output_select;

  this->ventilation_output_select_->set_write_callback([this](const std::string &value) {
    const bool connected = value == "Connected";
    if (this->ventilation_output_known_ && connected == this->ventilation_output_connected_) {
      this->ventilation_output_select_->publish_state(value);
      return;
    }
    this->on_ventilation_output_change(connected);
  });
}

void PanasonicPaci::set_remote_temperature_sensor_select(PanasonicPaciSelect *remote_temperature_sensor_select) {
  this->remote_temperature_sensor_select_ = remote_temperature_sensor_select;

  this->remote_temperature_sensor_select_->set_write_callback([this](const std::string &value) {
    const bool remote_controller = value == "Remote Controller";
    if (this->remote_temperature_sensor_known_ && remote_controller == this->remote_temperature_sensor_) {
      this->remote_temperature_sensor_select_->publish_state(value);
      return;
    }
    this->on_remote_temperature_sensor_change(remote_controller);
  });
}

void PanasonicPaci::set_temperature_unit_select(PanasonicPaciSelect *temperature_unit_select) {
  this->temperature_unit_select_ = temperature_unit_select;

  this->temperature_unit_select_->set_write_callback([this](const std::string &value) {
    const bool fahrenheit = value == "Fahrenheit";
    if (this->fahrenheit_unit_known_ && fahrenheit == this->fahrenheit_unit_) {
      this->temperature_unit_select_->publish_state(value);
      return;
    }
    this->on_fahrenheit_change(fahrenheit);
  });
}

void PanasonicPaci::set_target_temperature_sensor(sensor::Sensor *target_temperature_sensor) {
  this->target_temperature_sensor_ = target_temperature_sensor;
}

void PanasonicPaci::set_current_temperature_sensor(sensor::Sensor *current_temperature_sensor) {
  this->current_temperature_sensor_ = current_temperature_sensor;
}


void PanasonicPaci::set_indoor_model_text_sensor(text_sensor::TextSensor *sensor) { this->indoor_model_text_sensor_ = sensor; }
void PanasonicPaci::set_indoor_serial_text_sensor(text_sensor::TextSensor *sensor) { this->indoor_serial_text_sensor_ = sensor; }
void PanasonicPaci::set_outdoor_model_text_sensor(text_sensor::TextSensor *sensor) { this->outdoor_model_text_sensor_ = sensor; }
void PanasonicPaci::set_outdoor_serial_text_sensor(text_sensor::TextSensor *sensor) { this->outdoor_serial_text_sensor_ = sensor; }

void PanasonicPaci::log_packet(const std::vector<uint8_t> &data, bool outgoing) {
  if (outgoing) {
    ESP_LOGV(TAG, "TX: %s", format_hex_pretty(data).c_str());
  } else {
    ESP_LOGV(TAG, "RX: %s", format_hex_pretty(data).c_str());
  }
}

}  // namespace panasonic_paci
}  // namespace esphome
