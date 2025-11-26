#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_GUI_GUIINPUTHANDLER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_GUI_GUIINPUTHANDLER_H_

#include "InputHandler.h"
#include "Command.h"
#include <SFML/Graphics.hpp>
#include "additional/Other.h"
#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>

class GUIInputHandler : public InputHandler {
public:
    GUIInputHandler(sf::Window& window, const std::string& config_file = "keybinds_gui.cfg");
    ~GUIInputHandler() override = default;

    bool Initialize() override;
    std::unique_ptr<Command> GetCommand() override;
    const std::string control_legend() override;

    const std::map<sf::Keyboard::Key, CommandType>& key_bindings() const;
    void PrintKeyBindings() const;

    void handleEvent(const sf::Event& event);

private:
    sf::Window& window_;
    std::string config_file_;
    std::map<sf::Keyboard::Key, CommandType> key_bindings_;

    enum class MovementScheme { WASD, ARROWS };
    MovementScheme movement_scheme_;

    bool LoadFromFile();
    void SetDefaultBindings();
    bool SaveDefaultConfig() const;
    bool ValidateOrFixBindings();
    void SetMovementBindings();

    CommandType ParseCommand(const std::string& commandStr) const;
    std::string CommandToString(CommandType command) const;
    sf::Keyboard::Key ParseKey(const std::string& keyStr) const;
    std::string KeyToString(sf::Keyboard::Key key) const;
};

#endif