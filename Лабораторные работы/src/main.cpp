#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "controlGame/Game.h"
#include "controlGame/GameSettings.h"
#include "controlGame/controllerGame/GameController.h"
#include "controlGame/controllerGame/console/ConsoleRenderer.h"
#include "controlGame/controllerGame/GUI/GUIRenderer.h"
#include "controlGame/controllerGame/console/TerminalInputHandler.h"
#include "controlGame/controllerGame/GUI/GUIInputHandler.h"
#include "controlGame/controllerGame/SoundManager.h"
#include "controlGame/controllerGame/Renderer.h"


using ConsoleRenderStrategy = Renderer<ConsoleRenderer>;
using GUIRenderStrategy     = Renderer<GUIRenderer>;
using ConsoleController = GameController<TerminalInputHandler, ConsoleRenderStrategy>;
using GUIController = GameController<GUIInputHandler, GUIRenderStrategy>;

int main() {
    try {
        const bool audio_active = SoundManager::getInstance().LoadSounds();
        if (!audio_active) {
            std::cerr << "Предупреждение: Не удалось загрузить некоторые звуки\n";
        }

        GameSettings settings;
        settings.ShowSettingsDialog();
        Game game(settings);
        
        if (settings.interface_type() == InterfaceType::CONSOLE) {
            auto input_handler = std::make_unique<TerminalInputHandler>();
            auto renderer     = std::make_shared<ConsoleRenderStrategy>();
            auto controller = std::make_unique<ConsoleController>(game, std::move(input_handler), renderer);

            if (controller->Initialize()) {
                if (audio_active) SoundManager::getInstance().PlayBackgroundMusic();
                controller->Run();
            }

        } else {
            sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

            sf::RenderWindow window(
                sf::VideoMode(desktop.width-30, desktop.height-80),
                "Морской бой",
                sf::Style::Close
            );

            window.setVerticalSyncEnabled(true);

            auto renderer = std::make_shared<GUIRenderStrategy>(window, settings.cell_size());
            auto input_handler = std::make_unique<GUIInputHandler>(window, "keybinds_gui.cfg");
            auto controller = std::make_unique<GUIController>(game, std::move(input_handler), renderer);

            if (controller->Initialize()) {
                if (audio_active) SoundManager::getInstance().PlayBackgroundMusic();
                controller->Run();
            }
        } 

        SoundManager::getInstance().StopBackgroundMusic();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
