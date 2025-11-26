#ifndef BATTLESHIP_ABILITIES_SHELLING_H_
#define BATTLESHIP_ABILITIES_SHELLING_H_

#include <random>
#include <vector>
#include <utility>
#include "Ability.h"
#include "core/Ship.h"

class AbilityManager;

class Shelling : public Ability {
public:
    Shelling();
    void Use(AbilityManager& manager, int x, int y) override;

private:
    std::vector<std::pair<int,int>> CollectAliveTargets(AbilityManager& manager) const;
    static int RandomIndex(int n);
};

#endif
