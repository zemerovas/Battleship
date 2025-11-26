#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_INPUTHANDLER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_INPUTHANDLER_H_
#include "Command.h"
#include <memory>

class InputHandler {
public:
    virtual ~InputHandler() = default;
    virtual bool Initialize() = 0;
    virtual std::unique_ptr<Command> GetCommand() = 0;
    virtual const std::string control_legend() = 0;
    
};

#endif