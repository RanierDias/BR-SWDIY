#pragma once

#include "config/persisted_config_model.h"

bool save_config_to_eeprom(const PersistedConfig &config);
bool load_config_from_eeprom(PersistedConfig &config);
bool is_config_valid(const PersistedConfig &config);
