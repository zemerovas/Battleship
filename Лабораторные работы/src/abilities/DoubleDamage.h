#ifndef BATTLESHIP_ABILITIES_DOUBLEDAMAGE_H_
#define BATTLESHIP_ABILITIES_DOUBLEDAMAGE_H_

#include "Ability.h"

class AbilityManager;

class DoubleDamage : public Ability {
public:
    DoubleDamage();
    void Use(AbilityManager& manager, int x, int y) override;
};

#endif