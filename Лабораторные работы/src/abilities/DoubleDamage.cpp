#include "DoubleDamage.h"
#include "core/PlayingField.h"
#include "AbilityException.h"
#include "AbilityManager.h"
#include "additional/Other.h"

DoubleDamage::DoubleDamage() : Ability("Double Damage") {
    description_ = "Атака по кораблю нанесёт 2 урона (уничтожит сегмент).";
}

void DoubleDamage::Use(AbilityManager& manager, int x, int y) {
    try {
        coord_.first = x;
        coord_.second = y;
        manager.DamageEnemyField(x, y, 2);
    } catch (const std::exception& e) {
        throw AbilityApplicationException(name(), e.what());
    }
}