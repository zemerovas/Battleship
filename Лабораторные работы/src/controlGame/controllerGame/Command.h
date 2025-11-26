#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_COMMAND_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_COMMAND_H_
#include <string>

enum class CommandType {
    PAUSE,
    RESTART,
    EXIT,
    SAVE,
    LOAD,
    USE_ABILITY,
    ATTACK,
    REMOVE_SHIP,
    SET_NEW_FIELD,
    SET_NEW_SHIP_SIZES,
    TOGGLE_PLACEMENT_MODE,
    SHOW_SHIPS,
    HELP,
    STATS,
    MOVE_UP,  
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    UNKNOWN,
    PLACE_SHIP,
    ROTATE_SHIP,
    SET_1,
    SET_2,
    SET_3,
    SET_4, 
    SET_5,
    YES,
    NO
};

class Command {
public:
    Command(CommandType type, const std::string& description = "");
    CommandType type() const;
    std::string description() const;

private:
    CommandType type_;
    std::string description_;
};

#endif