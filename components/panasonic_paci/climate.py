from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, climate, sensor, switch, select, text_sensor

from . import panasonic_paci_ns

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "switch", "select", "text_sensor"]

PanasonicPaci = panasonic_paci_ns.class_(
    "PanasonicPaci", cg.Component, uart.UARTDevice, climate.Climate
)

panasonic_paci_wlan_ns = panasonic_paci_ns.namespace("WLAN")
PanasonicPaciWLAN = panasonic_paci_wlan_ns.class_("PanasonicPaciWLAN", PanasonicPaci)

PanasonicPaciSwitch = panasonic_paci_ns.class_(
    "PanasonicPaciSwitch", switch.Switch, cg.Component
)
PanasonicPaciSelect = panasonic_paci_ns.class_(
    "PanasonicPaciSelect", select.Select, cg.Component
)

CONF_WLAN = "wlan"
CONF_ECO_SWITCH = "eco_switch"
# PACi service/admin settings are selects, not switches.
# Old *_switch YAML keys are accepted as aliases so existing configs keep compiling.
CONF_VENTILATION_OUTPUT_SELECT = "ventilation_output_select"
CONF_REMOTE_TEMPERATURE_SENSOR_SELECT = "remote_temperature_sensor_select"
CONF_TEMPERATURE_UNIT_SELECT = "temperature_unit_select"

CONF_TARGET_TEMPERATURE_SENSOR = "target_temperature_sensor"
CONF_CURRENT_TEMPERATURE_SENSOR = "current_temperature_sensor"
CONF_OUTDOOR_TEMPERATURE_SENSOR = "outdoor_temperature"

CONF_INDOOR_MODEL = "indoor_model"
CONF_INDOOR_SERIAL = "indoor_serial"
CONF_OUTDOOR_MODEL = "outdoor_model"
CONF_OUTDOOR_SERIAL = "outdoor_serial"

SWITCH_SCHEMA = switch.switch_schema(PanasonicPaciSwitch).extend(cv.COMPONENT_SCHEMA)
VENTILATION_OUTPUT_SELECT_SCHEMA = select.select_schema(PanasonicPaciSelect).extend(cv.COMPONENT_SCHEMA)
REMOTE_TEMPERATURE_SENSOR_SELECT_SCHEMA = select.select_schema(PanasonicPaciSelect).extend(cv.COMPONENT_SCHEMA)
TEMPERATURE_UNIT_SELECT_SCHEMA = select.select_schema(PanasonicPaciSelect).extend(cv.COMPONENT_SCHEMA)

TEMPERATURE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

PANASONIC_WLAN_SCHEMA = {
    cv.Optional(CONF_ECO_SWITCH): SWITCH_SCHEMA,
    cv.Optional(CONF_VENTILATION_OUTPUT_SELECT): VENTILATION_OUTPUT_SELECT_SCHEMA,
    cv.Optional(CONF_REMOTE_TEMPERATURE_SENSOR_SELECT): REMOTE_TEMPERATURE_SENSOR_SELECT_SCHEMA,
    cv.Optional(CONF_TEMPERATURE_UNIT_SELECT): TEMPERATURE_UNIT_SELECT_SCHEMA,
    cv.Optional(CONF_TARGET_TEMPERATURE_SENSOR): TEMPERATURE_SENSOR_SCHEMA,
    cv.Optional(CONF_CURRENT_TEMPERATURE_SENSOR): TEMPERATURE_SENSOR_SCHEMA,
    cv.Optional(CONF_OUTDOOR_TEMPERATURE_SENSOR): TEMPERATURE_SENSOR_SCHEMA,
    cv.Optional(CONF_INDOOR_MODEL): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_INDOOR_SERIAL): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_OUTDOOR_MODEL): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_OUTDOOR_SERIAL): text_sensor.text_sensor_schema(),
}

CONFIG_SCHEMA = cv.typed_schema(
    {
        CONF_WLAN: climate.climate_schema(PanasonicPaciWLAN)
        .extend(PANASONIC_WLAN_SCHEMA)
        .extend(uart.UART_DEVICE_SCHEMA),
    }
)


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_controller_address(0xE0))

    if CONF_ECO_SWITCH in config:
        conf = config[CONF_ECO_SWITCH]
        eco_switch = await switch.new_switch(conf)
        await cg.register_component(eco_switch, conf)
        cg.add(var.set_eco_switch(eco_switch))

    if CONF_VENTILATION_OUTPUT_SELECT in config:
        conf = config[CONF_VENTILATION_OUTPUT_SELECT]
        ventilation_output_select = await select.new_select(conf, options=["Not Connected", "Connected"])
        await cg.register_component(ventilation_output_select, conf)
        cg.add(var.set_ventilation_output_select(ventilation_output_select))

    if CONF_REMOTE_TEMPERATURE_SENSOR_SELECT in config:
        conf = config[CONF_REMOTE_TEMPERATURE_SENSOR_SELECT]
        remote_temperature_sensor_select = await select.new_select(conf, options=["Main Unit", "Remote Controller"])
        await cg.register_component(remote_temperature_sensor_select, conf)
        cg.add(var.set_remote_temperature_sensor_select(remote_temperature_sensor_select))

    if CONF_TEMPERATURE_UNIT_SELECT in config:
        conf = config[CONF_TEMPERATURE_UNIT_SELECT]
        temperature_unit_select = await select.new_select(conf, options=["Celsius", "Fahrenheit"])
        await cg.register_component(temperature_unit_select, conf)
        cg.add(var.set_temperature_unit_select(temperature_unit_select))

    if CONF_TARGET_TEMPERATURE_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_TARGET_TEMPERATURE_SENSOR])
        cg.add(var.set_target_temperature_sensor(sens))

    if CONF_CURRENT_TEMPERATURE_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_TEMPERATURE_SENSOR])
        cg.add(var.set_current_temperature_sensor(sens))


    if CONF_OUTDOOR_TEMPERATURE_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_OUTDOOR_TEMPERATURE_SENSOR])
        cg.add(var.set_outdoor_temperature_sensor(sens))

    if CONF_INDOOR_MODEL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_INDOOR_MODEL])
        cg.add(var.set_indoor_model_text_sensor(sens))

    if CONF_INDOOR_SERIAL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_INDOOR_SERIAL])
        cg.add(var.set_indoor_serial_text_sensor(sens))

    if CONF_OUTDOOR_MODEL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_OUTDOOR_MODEL])
        cg.add(var.set_outdoor_model_text_sensor(sens))

    if CONF_OUTDOOR_SERIAL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_OUTDOOR_SERIAL])
        cg.add(var.set_outdoor_serial_text_sensor(sens))
