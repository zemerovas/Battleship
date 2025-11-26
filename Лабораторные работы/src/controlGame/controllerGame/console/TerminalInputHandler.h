#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_CONSOLE_TERMINALINPUTHANDLER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_CONSOLE_TERMINALINPUTHANDLER_H_

#include "InputHandler.h"
#include "Command.h"

#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include "additional/Other.h"


class TerminalInputHandler : public InputHandler {
public:
    explicit TerminalInputHandler(const std::string& configFile = "keybinds.cfg");
    ~TerminalInputHandler() override;

    bool Initialize() override;
    std::unique_ptr<Command> GetCommand() override;
    void PrintKeyBindings() const;

    const std::map<char, CommandType>& key_bindings() const;
    const std::string control_legend() override;
    
private:
    std::string config_file_;
    enum class MovementScheme { WASD, ARROWS };
    MovementScheme movement_scheme_ ;

    struct termios original_termios_;
    std::map<char, CommandType> key_bindings_;

    // анти-дребезг перемещения
    std::chrono::steady_clock::time_point last_move_{};
    int repeat_ms_ = 100;

    bool SetUpTerminal();
    void RestoreTerminal();
    bool loadFromFile();
    void SetDefaultBindings();
    bool SaveDefaultConfig() const;
    bool ValidateOrFixBindings();
    void SetMovementBindings();
    std::unique_ptr<Command> CreateMoveCommand(CommandType command);

    CommandType ParseCommand(const std::string& commandStr) const;
    std::string CommandToString(CommandType command) const;
    char ParseTerminalKey(const std::string& key_str) const;
    std::string KeyToDisplayString(char key) const;

};


#endif