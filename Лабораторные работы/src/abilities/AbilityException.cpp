#include "AbilityException.h"
#include <string>

AbilityException::AbilityException(const std::string& message)
    : std::runtime_error("Ошибка способности: " + message) {}


EmptyQueueException::EmptyQueueException(const std::string& msg): AbilityException(msg) {}

AbilityApplicationException::AbilityApplicationException(
    const std::string& ability_name, const std::string& reason)
    : AbilityException("Не удалось применить способность '" + ability_name + "' - " + reason),
      ability_name_(ability_name) {}
