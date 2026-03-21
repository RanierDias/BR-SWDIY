#include "hw/eeprom_store.h"

#include <EEPROM.h>

namespace
{
    constexpr int EEPROM_START_ADDRESS = 0;
    constexpr uint16_t PERSISTED_CONFIG_MAGIC = 0x4252;
    constexpr uint8_t PERSISTED_CONFIG_VERSION = 1;
}

bool is_config_valid(const PersistedConfig &config)
{
    return config.magic == PERSISTED_CONFIG_MAGIC && config.version == PERSISTED_CONFIG_VERSION;
}

bool save_config_to_eeprom(const PersistedConfig &config)
{
    EEPROM.put(EEPROM_START_ADDRESS, config);
    return true;
}

bool load_config_from_eeprom(PersistedConfig &config)
{
    EEPROM.get(EEPROM_START_ADDRESS, config);
    return is_config_valid(config);
}
