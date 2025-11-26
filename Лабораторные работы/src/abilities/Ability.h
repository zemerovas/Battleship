#ifndef BATTLESHIP_ABILITIES_ABILITY_H_
#define BATTLESHIP_ABILITIES_ABILITY_H_

#include <string>
#include <utility>
#include "controlGame/Game.h"
#include "controlGame/Result.h"

class AbilityManager;

class Ability {
protected:
    std::string name_;
    std::string description_;
    std::pair<int, int> coord_;

public:
    Ability(const std::string& ability_name);
    virtual ~Ability() = default;
    virtual void Use(AbilityManager& manager, int x, int y) = 0;
    std::string name() const;
    std::string description() const;
    std::pair<int, int> coordinates() const;
};

#endif