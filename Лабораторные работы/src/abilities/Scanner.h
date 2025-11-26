#ifndef BATTLESHIP_ABILITIES_SCANNER_H_
#define BATTLESHIP_ABILITIES_SCANNER_H_

#include <vector>
#include <utility>
#include "Ability.h"
#include "core/PlayingField.h"
#include "core/Ship.h"
#include "additional/Other.h"

struct Position;
class AbilityManager;

class Scanner : public Ability { 
private:
    int scan_range_;

public:
    Scanner();
    void Use(AbilityManager& manager, int x, int y) override;
    std::vector<std::pair<Position, bool>> ScanArea(AbilityManager& manager, Position center) const;
};

#endif