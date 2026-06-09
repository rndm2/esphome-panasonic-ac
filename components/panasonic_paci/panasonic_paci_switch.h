#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome {
namespace panasonic_paci {

class PanasonicPaciSwitch : public switch_::Switch, public Component {
 protected:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace panasonic_paci
}  // namespace esphome
