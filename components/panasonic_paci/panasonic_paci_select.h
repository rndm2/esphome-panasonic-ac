#pragma once

#include <functional>
#include <string>

#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

namespace esphome {
namespace panasonic_paci {

class PanasonicPaciSelect : public select::Select, public Component {
 public:
  void set_write_callback(std::function<void(const std::string &)> &&callback) { this->write_callback_ = std::move(callback); }

 protected:
  void control(const std::string &value) override {
    if (this->write_callback_) {
      this->write_callback_(value);
    } else {
      this->publish_state(value);
    }
  }

  std::function<void(const std::string &)> write_callback_;
};

}  // namespace panasonic_paci
}  // namespace esphome
