import esphome.codegen as cg

CODEOWNERS = ["@rndm2"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "switch", "select", "text_sensor"]

panasonic_ac_ns = cg.esphome_ns.namespace("panasonic_ac")
