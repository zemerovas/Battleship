#ifndef BATTLESHIP_CONTROLGAME_RESULT_H_
#define BATTLESHIP_CONTROLGAME_RESULT_H_
#include <string>

struct AttackResult {
    int hit;
    int x;
    int y;
};

struct AbilityResult {
    std::string ability_name;
    int x;
    int y;
};

#endif