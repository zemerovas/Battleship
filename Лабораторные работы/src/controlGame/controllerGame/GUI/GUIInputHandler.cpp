#include "GUIInputHandler.h"

#include <optional>
#include <set>

GUIInputHandler::GUIInputHandler(sf::Window& window, const std::string& config_file)
    : window_(window), config_file_(config_file), movement_scheme_(MovementScheme::ARROWS) {}

bool GUIInputHandler::Initialize() {
    if (!LoadFromFile()) {
        std::cout << "Не удалось загрузить настройки из файла '" << config_file_
                  << "'. Используются настройки по умолчанию.\n";
        SetDefaultBindings(); 
    }

    if (!ValidateOrFixBindings()) {
        std::cout << "Некоторые назначения клавиш были автоматически исправлены.\n";
        SaveDefaultConfig();
    }

    SetMovementBindings();

    std::cout << "Управление GUI успешно настроено. Загружено "
              << key_bindings_.size() << " назначений клавиш.\n";
    PrintKeyBindings();
    return true;
}


void GUIInputHandler::SetMovementBindings() {
    std::vector<sf::Keyboard::Key> to_remove;
    for (const auto& p : key_bindings_) {
        if (p.second >= CommandType::MOVE_UP && p.second <= CommandType::MOVE_RIGHT)
            to_remove.push_back(p.first);
    }
    for (auto k : to_remove) key_bindings_.erase(k);

    if (movement_scheme_ == MovementScheme::WASD) {
        key_bindings_[sf::Keyboard::W] = CommandType::MOVE_UP;
        key_bindings_[sf::Keyboard::S] = CommandType::MOVE_DOWN;
        key_bindings_[sf::Keyboard::A] = CommandType::MOVE_LEFT;
        key_bindings_[sf::Keyboard::D] = CommandType::MOVE_RIGHT;
    } else {
        key_bindings_[sf::Keyboard::Up]    = CommandType::MOVE_UP;
        key_bindings_[sf::Keyboard::Down]  = CommandType::MOVE_DOWN;
        key_bindings_[sf::Keyboard::Left]  = CommandType::MOVE_LEFT;
        key_bindings_[sf::Keyboard::Right] = CommandType::MOVE_RIGHT;
    }
}


std::unique_ptr<Command> GUIInputHandler::GetCommand() {
    static std::optional<Command> last_command;  

    if (last_command) {
        auto cmd = std::make_unique<Command>(*last_command);
        last_command.reset();
        return cmd;
    }

    sf::Event event;
    while (window_.pollEvent(event)) {
        if (!window_.hasFocus()) continue;

        if (event.type == sf::Event::Closed) {
            last_command = Command(CommandType::EXIT);
            continue;
        }
        
        if (event.type == sf::Event::KeyPressed) {
            // Обработка движения в зависимости от схемы
            if (movement_scheme_ == MovementScheme::WASD) {
                CommandType wasd_command = CommandType::UNKNOWN;
                switch (event.key.code) {
                    case sf::Keyboard::W: wasd_command = CommandType::MOVE_UP; break;
                    case sf::Keyboard::S: wasd_command = CommandType::MOVE_DOWN; break;
                    case sf::Keyboard::A: wasd_command = CommandType::MOVE_LEFT; break;
                    case sf::Keyboard::D: wasd_command = CommandType::MOVE_RIGHT; break;
                    default: break;
                }
                if (wasd_command != CommandType::UNKNOWN) {
                    last_command = Command(wasd_command);
                    continue;
                }
            } else { // ARROWS схема
                CommandType arrow_command = CommandType::UNKNOWN;
                switch (event.key.code) {
                    case sf::Keyboard::Up:    arrow_command = CommandType::MOVE_UP; break;
                    case sf::Keyboard::Down:  arrow_command = CommandType::MOVE_DOWN; break;
                    case sf::Keyboard::Left:  arrow_command = CommandType::MOVE_LEFT; break;
                    case sf::Keyboard::Right: arrow_command = CommandType::MOVE_RIGHT; break;
                    default: break;
                }
                if (arrow_command != CommandType::UNKNOWN) {
                    last_command = Command(arrow_command);
                    continue;
                }
            }
            
            // Остальные команды из конфига
            auto it = key_bindings_.find(event.key.code);
            if (it != key_bindings_.end()) {
                last_command = Command(it->second);
            }
        }
    }

    if (last_command) {
        auto cmd = std::make_unique<Command>(*last_command);
        last_command.reset();
        return cmd;
    }

    return nullptr;
}


bool GUIInputHandler::LoadFromFile() {
    std::ifstream file(config_file_);
    if (!file.is_open()) {
        return false;
    }

    key_bindings_.clear();
    movement_scheme_ = MovementScheme::ARROWS;

    std::string line;
    int line_number = 0;
    bool had_conflict = false;

    while (std::getline(file, line)) {
        ++line_number;
        Trim(line);
        if (line.empty() || IsComment(line)) continue;

        // --- movement_scheme ---
        if (line.find("movement_scheme") != std::string::npos) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string scheme = line.substr(pos + 1);
                Trim(scheme);
                if (scheme == "WASD")       movement_scheme_ = MovementScheme::WASD;
                else if (scheme == "ARROWS") movement_scheme_ = MovementScheme::ARROWS;
            }
            continue;
        }

        size_t delim = line.find('=');
        if (delim == std::string::npos || delim == 0) {
            std::cerr << "Ошибка в строке " << line_number << ": нет '=' или клавиши\n";
            continue;
        }

        std::string key_str = line.substr(0, delim);
        std::string cmd_str = line.substr(delim + 1);
        Trim(key_str); Trim(cmd_str);

        CommandType cmd = ParseCommand(cmd_str);
        if (cmd == CommandType::UNKNOWN) {
            std::cerr << "Ошибка в строке " << line_number << ": неизвестная команда '" << cmd_str << "'\n";
            continue;
        }

        if (cmd >= CommandType::MOVE_UP && cmd <= CommandType::MOVE_RIGHT) {
            std::cerr << "Предупреждение в строке " << line_number << ": команды движения нельзя назначать вручную\n";
            continue;
        }

        sf::Keyboard::Key original_key = ParseKey(key_str);
        if (original_key == sf::Keyboard::Unknown) {
            std::cerr << "Ошибка в строке " << line_number << ": неизвестная клавиша '" << key_str << "'\n";
            continue;
        }

        // Проверка конфликта с клавишами движения
        const std::vector<sf::Keyboard::Key> wasd_keys  = {sf::Keyboard::W, sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D};
        const std::vector<sf::Keyboard::Key> arrow_keys = {sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right};
        const auto& reserved = (movement_scheme_ == MovementScheme::WASD) ? wasd_keys : arrow_keys;

        bool conflict = (std::find(reserved.begin(), reserved.end(), original_key) != reserved.end());

        if (conflict) {
            std::cout << "Конфликт в строке " << line_number
                      << ": клавиша '" << KeyToString(original_key)
                      << "' зарезервирована под движение ("
                      << (movement_scheme_ == MovementScheme::WASD ? "WASD" : "СТРЕЛКИ")
                      << "). Команда '" << cmd_str << "' будет переназначена автоматически.\n";

            // Поиск безопасной и свободной клавиши
            sf::Keyboard::Key new_key = sf::Keyboard::Unknown;

            auto is_safe_and_free = [&](sf::Keyboard::Key k) -> bool {
                if (key_bindings_.find(k) != key_bindings_.end()) return false;
                if (std::find(reserved.begin(), reserved.end(), k) != reserved.end()) return false;
                return true;
            };

            auto find_free_digit = [&]() -> sf::Keyboard::Key {
                for (int i = 0; i <= 9; ++i) {
                    sf::Keyboard::Key k = static_cast<sf::Keyboard::Key>(sf::Keyboard::Num0 + i);
                    if (is_safe_and_free(k)) return k;
                }
                return sf::Keyboard::Unknown;
            };

            auto find_free_letter = [&]() -> sf::Keyboard::Key {
                for (char c = 'A'; c <= 'Z'; ++c) {
                    sf::Keyboard::Key k = static_cast<sf::Keyboard::Key>(sf::Keyboard::A + (c - 'A'));
                    if (is_safe_and_free(k)) return k;
                }
                return sf::Keyboard::Unknown;
            };

            if (cmd >= CommandType::SET_1 && cmd <= CommandType::SET_5) {
                new_key = find_free_digit();
                if (new_key == sf::Keyboard::Unknown) new_key = find_free_letter();
            } else {
                new_key = find_free_letter();
                if (new_key == sf::Keyboard::Unknown) new_key = find_free_digit();
            }

            if (new_key != sf::Keyboard::Unknown) {
                key_bindings_[new_key] = cmd;
                std::cout << "→ '" << cmd_str << "' переназначена на '" << KeyToString(new_key) << "'\n";
                had_conflict = true;
            } else {
                std::cerr << "Критическая ошибка: не удалось найти безопасную клавишу для '" << cmd_str << "'\n";
            }
        } else {
            // Нет конфликта — просто записываем
            if (key_bindings_.find(original_key) != key_bindings_.end()) {
                std::cout << "Предупреждение в строке " << line_number
                          << ": клавиша '" << KeyToString(original_key) << "' перезаписана\n";
            }
            key_bindings_[original_key] = cmd;
        }
    }

    file.close();

    if (had_conflict) {
        std::cout << "Настройки GUI загружены с автоматическим исправлением конфликтов.\n";
        SaveDefaultConfig(); 
    }

    return true;
}

void GUIInputHandler::SetDefaultBindings() {
    key_bindings_.clear();

    key_bindings_[sf::Keyboard::Space]   = CommandType::ATTACK;
    key_bindings_[sf::Keyboard::P]       = CommandType::PLACE_SHIP;
    key_bindings_[sf::Keyboard::R]       = CommandType::ROTATE_SHIP;
    key_bindings_[sf::Keyboard::Q]       = CommandType::EXIT;
    key_bindings_[sf::Keyboard::U]       = CommandType::USE_ABILITY;
    key_bindings_[sf::Keyboard::H]       = CommandType::HELP;
    key_bindings_[sf::Keyboard::T]       = CommandType::STATS;
    key_bindings_[sf::Keyboard::L]       = CommandType::LOAD;
    key_bindings_[sf::Keyboard::F2]      = CommandType::SAVE;
    key_bindings_[sf::Keyboard::F5]      = CommandType::RESTART;
    key_bindings_[sf::Keyboard::Escape]  = CommandType::PAUSE;
    key_bindings_[sf::Keyboard::E]       = CommandType::SET_NEW_FIELD;
    key_bindings_[sf::Keyboard::Z]       = CommandType::SET_NEW_SHIP_SIZES;
    key_bindings_[sf::Keyboard::Num1]    = CommandType::SET_1;
    key_bindings_[sf::Keyboard::Num2]    = CommandType::SET_2;
    key_bindings_[sf::Keyboard::Num3]    = CommandType::SET_3;
    key_bindings_[sf::Keyboard::Num4]    = CommandType::SET_4;
    key_bindings_[sf::Keyboard::Num5]    = CommandType::SET_5;
    key_bindings_[sf::Keyboard::Y]       = CommandType::YES;
    key_bindings_[sf::Keyboard::N]       = CommandType::NO;

    if (movement_scheme_ == MovementScheme::WASD) {
        key_bindings_[sf::Keyboard::I] = CommandType::SHOW_SHIPS;
        key_bindings_[sf::Keyboard::O] = CommandType::REMOVE_SHIP;
        key_bindings_[sf::Keyboard::J] = CommandType::TOGGLE_PLACEMENT_MODE;
    } else {
        key_bindings_[sf::Keyboard::S] = CommandType::SHOW_SHIPS;
        key_bindings_[sf::Keyboard::D] = CommandType::REMOVE_SHIP;
        key_bindings_[sf::Keyboard::A] = CommandType::TOGGLE_PLACEMENT_MODE;
    }
}


bool GUIInputHandler::SaveDefaultConfig() const {
    std::ofstream file(config_file_);
    if (!file.is_open()) return false;

    file << "# КОНФИГУРАЦИЯ УПРАВЛЕНИЯ - МОРСКОЙ БОЙ (GUI)\n";
    file << "# Формат: клавиша = команда\n";
    file << "# Регистр клавиш не важен. Не назначайте разные команды на одну клавишу.\n\n";

    file << "# Выбор схемы управления (WASD или ARROWS)\n";
    file << "# В режиме WASD: W,A,S,D = движение, Стрелки = не назначены\n";
    file << "# В режиме ARROWS: Стрелки = движение, W,A,S,D = не назначены\n";
    std::string current_scheme = (movement_scheme_ == MovementScheme::WASD) ? "WASD" : "ARROWS";
    file << "movement_scheme = " << current_scheme << "\n\n";

    file << "# Основные команды:\n";
    file << "# Вы можете изменить клавиши, но не меняйте названия команд после '='\n";
    file << "# Доступные клавиши: A-Z, 0-9, F1-F15, Space, Enter, Escape, Arrow keys, etc.\n\n";


    for (const auto& [key, command] : key_bindings_) {
        file << KeyToString(key) << " = " << CommandToString(command) << "\n";
    }

    file << "\n# ПОДСКАЗКИ:\n";
    file << "# - После изменения файла перезапустите игру\n";
    file << "# - Для сброса настроек удалите этот файл\n";
    file << "# - Команды движения настраиваются через movement_scheme\n";
    file.close();
    return true;
}


bool GUIInputHandler::ValidateOrFixBindings() {
    bool fixed = false;
    std::set<sf::Keyboard::Key> occupied;
    for (const auto& p : key_bindings_) occupied.insert(p.first);

    const std::vector<sf::Keyboard::Key> move_keys = (movement_scheme_ == MovementScheme::WASD)
        ? std::vector<sf::Keyboard::Key>{sf::Keyboard::W,sf::Keyboard::A,sf::Keyboard::S,sf::Keyboard::D}
        : std::vector<sf::Keyboard::Key>{sf::Keyboard::Up,sf::Keyboard::Down,sf::Keyboard::Left,sf::Keyboard::Right};

    auto is_reserved = [&](sf::Keyboard::Key k) {
        return std::find(move_keys.begin(), move_keys.end(), k) != move_keys.end();
    };

    auto free_digit = [&]() -> sf::Keyboard::Key {
        for (int i = 0; i <= 9; ++i) {
            sf::Keyboard::Key k = static_cast<sf::Keyboard::Key>(sf::Keyboard::Num0 + i);
            if (!occupied.count(k) && !is_reserved(k)) return k;
        }
        return sf::Keyboard::Unknown;
    };

    auto free_letter = [&]() -> sf::Keyboard::Key {
        for (char c = 'A'; c <= 'Z'; ++c) {
            sf::Keyboard::Key k = static_cast<sf::Keyboard::Key>(sf::Keyboard::A + (c - 'A'));
            if (!occupied.count(k) && !is_reserved(k)) return k;
        }
        return sf::Keyboard::Unknown;
    };

    struct Req { CommandType cmd; sf::Keyboard::Key pref; };
    const std::vector<Req> required = {
        {CommandType::ATTACK,              sf::Keyboard::Space},
        {CommandType::PLACE_SHIP,          sf::Keyboard::P},
        {CommandType::ROTATE_SHIP,         sf::Keyboard::R},
        {CommandType::EXIT,                sf::Keyboard::Q},
        {CommandType::USE_ABILITY,         sf::Keyboard::U},
        {CommandType::HELP,                sf::Keyboard::H},
        {CommandType::STATS,               sf::Keyboard::T},
        {CommandType::LOAD,                sf::Keyboard::L},
        {CommandType::SAVE,                sf::Keyboard::F2},
        {CommandType::RESTART,             sf::Keyboard::F5},
        {CommandType::PAUSE,               sf::Keyboard::Escape},
        {CommandType::SET_NEW_FIELD,       sf::Keyboard::E},
        {CommandType::SET_NEW_SHIP_SIZES,  sf::Keyboard::Z},
        {CommandType::YES,                 sf::Keyboard::Y},
        {CommandType::NO,                  sf::Keyboard::N},
        {CommandType::SET_1,               sf::Keyboard::Num1},
        {CommandType::SET_2,               sf::Keyboard::Num2},
        {CommandType::SET_3,               sf::Keyboard::Num3},
        {CommandType::SET_4,               sf::Keyboard::Num4},
        {CommandType::SET_5,               sf::Keyboard::Num5},
        {CommandType::SHOW_SHIPS,          movement_scheme_ == MovementScheme::WASD ? sf::Keyboard::I : sf::Keyboard::S},
        {CommandType::REMOVE_SHIP,         movement_scheme_ == MovementScheme::WASD ? sf::Keyboard::O : sf::Keyboard::D},
        {CommandType::TOGGLE_PLACEMENT_MODE, movement_scheme_ == MovementScheme::WASD ? sf::Keyboard::J : sf::Keyboard::A}
    };

    for (const auto& r : required) {
        bool exists = false;
        for (const auto& p : key_bindings_) if (p.second == r.cmd) { exists = true; break; }
        if (exists) continue;

        sf::Keyboard::Key key = r.pref;
        if (occupied.count(key) || is_reserved(key)) {
            if (r.cmd >= CommandType::SET_1 && r.cmd <= CommandType::SET_5) {
                key = free_digit();
                if (key == sf::Keyboard::Unknown) key = free_letter();
            } else {
                key = free_letter();
                if (key == sf::Keyboard::Unknown) key = free_digit();
            }
        }
        if (key == sf::Keyboard::Unknown) {
            std::cerr << "Не удалось найти свободную клавишу для " << CommandToString(r.cmd) << "\n";
            return false;
        }

        key_bindings_[key] = r.cmd;
        occupied.insert(key);
        fixed = true;
        std::cout << "Автоназначение: " << CommandToString(r.cmd) << " → " << KeyToString(key) << "\n";
    }
    return !fixed;
}

CommandType GUIInputHandler::ParseCommand(const std::string& command_str) const {
    if (command_str == "MOVE_UP")                    return CommandType::MOVE_UP;
    if (command_str == "MOVE_DOWN")                  return CommandType::MOVE_DOWN;
    if (command_str == "MOVE_LEFT")                  return CommandType::MOVE_LEFT;
    if (command_str == "MOVE_RIGHT")                 return CommandType::MOVE_RIGHT;
    if (command_str == "ATTACK")                     return CommandType::ATTACK;
    if (command_str == "REMOVE_SHIP")                return CommandType::REMOVE_SHIP;
    if (command_str == "PLACE_SHIP")                 return CommandType::PLACE_SHIP;
    if (command_str == "SHOW_SHIPS")                 return CommandType::SHOW_SHIPS; 
    if (command_str == "ROTATE_SHIP")                return CommandType::ROTATE_SHIP;
    if (command_str == "EXIT")                       return CommandType::EXIT;
    if (command_str == "RESTART")                    return CommandType::RESTART;
    if (command_str == "HELP")                       return CommandType::HELP;
    if (command_str == "USE_ABILITY")                return CommandType::USE_ABILITY;
    if (command_str == "STATS")                      return CommandType::STATS;
    if (command_str == "LOAD")                       return CommandType::LOAD;
    if (command_str == "SAVE")                       return CommandType::SAVE;
    if (command_str == "PAUSE")                      return CommandType::PAUSE;
    if (command_str == "SET_NEW_FIELD")              return CommandType::SET_NEW_FIELD;
    if (command_str == "SET_NEW_SHIP_SIZES")         return CommandType::SET_NEW_SHIP_SIZES;
    if (command_str == "TOGGLE_PLACEMENT_MODE")      return CommandType::TOGGLE_PLACEMENT_MODE;
    if (command_str == "SET_1")                      return CommandType::SET_1;
    if (command_str == "SET_2")                      return CommandType::SET_2;
    if (command_str == "SET_3")                      return CommandType::SET_3;
    if (command_str == "SET_4")                      return CommandType::SET_4;
    if (command_str == "SET_5")                      return CommandType::SET_5;
    if (command_str == "YES")                        return CommandType::YES;
    if (command_str == "NO")                         return CommandType::NO;

    return CommandType::UNKNOWN;
}


std::string GUIInputHandler::CommandToString(CommandType command) const {
    switch (command) {
        case CommandType::MOVE_UP:     return "MOVE_UP";
        case CommandType::MOVE_DOWN:   return "MOVE_DOWN";
        case CommandType::MOVE_LEFT:   return "MOVE_LEFT";
        case CommandType::MOVE_RIGHT:  return "MOVE_RIGHT";
        case CommandType::ATTACK:      return "ATTACK";
        case CommandType::REMOVE_SHIP: return "REMOVE_SHIP";
        case CommandType::SHOW_SHIPS:  return "SHOW_SHIPS";
        case CommandType::PLACE_SHIP:  return "PLACE_SHIP";
        case CommandType::ROTATE_SHIP: return "ROTATE_SHIP";
        case CommandType::EXIT:        return "EXIT";
        case CommandType::RESTART:     return "RESTART";
        case CommandType::HELP:        return "HELP";
        case CommandType::USE_ABILITY: return "USE_ABILITY";
        case CommandType::STATS:       return "STATS";
        case CommandType::LOAD:        return "LOAD";
        case CommandType::SAVE:        return "SAVE";
        case CommandType::PAUSE:       return "PAUSE";
        case CommandType::SET_NEW_FIELD:        return "SET_NEW_FIELD";
        case CommandType::SET_NEW_SHIP_SIZES:   return "SET_NEW_SHIP_SIZES";
        case CommandType::TOGGLE_PLACEMENT_MODE:    return "TOGGLE_PLACEMENT_MODE";
        case CommandType::SET_1: return "SET_1";
        case CommandType::SET_2: return "SET_2";
        case CommandType::SET_3: return "SET_3";
        case CommandType::SET_4: return "SET_4";
        case CommandType::SET_5: return "SET_5";
        case CommandType::YES: return "YES";
        case CommandType::NO: return "NO";
        default:                       return "UNKNOWN";
    }
}

sf::Keyboard::Key GUIInputHandler::ParseKey(const std::string& key_str) const {
    std::string lower_key = key_str;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);

    // Буквы
    if (lower_key.length() == 1 && lower_key[0] >= 'a' && lower_key[0] <= 'z') {
        return static_cast<sf::Keyboard::Key>(sf::Keyboard::A + (lower_key[0] - 'a'));
    }

    // Цифры
    if (lower_key.length() == 1 && lower_key[0] >= '0' && lower_key[0] <= '9') {
        return static_cast<sf::Keyboard::Key>(sf::Keyboard::Num0 + (lower_key[0] - '0'));
    }

    // Специальные клавиши
    if (lower_key == "space") return sf::Keyboard::Space;
    if (lower_key == "enter" || lower_key == "return") return sf::Keyboard::Enter;
    if (lower_key == "escape" || lower_key == "esc") return sf::Keyboard::Escape;
    if (lower_key == "backspace") return sf::Keyboard::Backspace;
    if (lower_key == "tab") return sf::Keyboard::Tab;
    
    // Стрелки
    if (lower_key == "up") return sf::Keyboard::Up;
    if (lower_key == "down") return sf::Keyboard::Down;
    if (lower_key == "left") return sf::Keyboard::Left;
    if (lower_key == "right") return sf::Keyboard::Right;
    
    // Функциональные клавиши
    for (int i = 1; i <= 15; i++) {
        if (lower_key == "f" + std::to_string(i)) {
            return static_cast<sf::Keyboard::Key>(sf::Keyboard::F1 + (i - 1));
        }
    }

    
    // Модификаторы
    if (lower_key == "lshift" || lower_key == "leftshift") return sf::Keyboard::LShift;
    if (lower_key == "rshift" || lower_key == "rightshift") return sf::Keyboard::RShift;
    if (lower_key == "lctrl" || lower_key == "leftctrl") return sf::Keyboard::LControl;
    if (lower_key == "rctrl" || lower_key == "rightctrl") return sf::Keyboard::RControl;
    if (lower_key == "lalt" || lower_key == "leftalt") return sf::Keyboard::LAlt;
    if (lower_key == "ralt" || lower_key == "rightalt") return sf::Keyboard::RAlt;
    if (lower_key == "lsystem" || lower_key == "leftsystem") return sf::Keyboard::LSystem;
    if (lower_key == "rsystem" || lower_key == "rightsystem") return sf::Keyboard::RSystem;

    

    return sf::Keyboard::Unknown;
}



std::string GUIInputHandler::KeyToString(sf::Keyboard::Key key) const {
    switch (key) {
        // Буквы
        case sf::Keyboard::A: return "A";
        case sf::Keyboard::B: return "B";
        case sf::Keyboard::C: return "C";
        case sf::Keyboard::D: return "D";
        case sf::Keyboard::E: return "E";
        case sf::Keyboard::F: return "F";
        case sf::Keyboard::G: return "G";
        case sf::Keyboard::H: return "H";
        case sf::Keyboard::I: return "I";
        case sf::Keyboard::J: return "J";
        case sf::Keyboard::K: return "K";
        case sf::Keyboard::L: return "L";
        case sf::Keyboard::M: return "M";
        case sf::Keyboard::N: return "N";
        case sf::Keyboard::O: return "O";
        case sf::Keyboard::P: return "P";
        case sf::Keyboard::Q: return "Q";
        case sf::Keyboard::R: return "R";
        case sf::Keyboard::S: return "S";
        case sf::Keyboard::T: return "T";
        case sf::Keyboard::U: return "U";
        case sf::Keyboard::V: return "V";
        case sf::Keyboard::W: return "W";
        case sf::Keyboard::X: return "X";
        case sf::Keyboard::Y: return "Y";
        case sf::Keyboard::Z: return "Z";
        
        // Цифры
        case sf::Keyboard::Num0: return "0";
        case sf::Keyboard::Num1: return "1";
        case sf::Keyboard::Num2: return "2";
        case sf::Keyboard::Num3: return "3";
        case sf::Keyboard::Num4: return "4";
        case sf::Keyboard::Num5: return "5";
        case sf::Keyboard::Num6: return "6";
        case sf::Keyboard::Num7: return "7";
        case sf::Keyboard::Num8: return "8";
        case sf::Keyboard::Num9: return "9";
        
        // Функциональные клавиши
        case sf::Keyboard::F1: return "F1";
        case sf::Keyboard::F2: return "F2";
        case sf::Keyboard::F3: return "F3";
        case sf::Keyboard::F4: return "F4";
        case sf::Keyboard::F5: return "F5";
        case sf::Keyboard::F6: return "F6";
        case sf::Keyboard::F7: return "F7";
        case sf::Keyboard::F8: return "F8";
        case sf::Keyboard::F9: return "F9";
        case sf::Keyboard::F10: return "F10";
        case sf::Keyboard::F11: return "F11";
        case sf::Keyboard::F12: return "F12";
        case sf::Keyboard::F13: return "F13";
        case sf::Keyboard::F14: return "F14";
        case sf::Keyboard::F15: return "F15";
        
        // Специальные клавиши
        case sf::Keyboard::Space: return "Space";
        case sf::Keyboard::Enter: return "Enter";
        case sf::Keyboard::Escape: return "Escape";
        case sf::Keyboard::Backspace: return "Backspace";
        case sf::Keyboard::Tab: return "Tab";
        
        // Стрелки
        case sf::Keyboard::Up: return "Up";
        case sf::Keyboard::Down: return "Down";
        case sf::Keyboard::Left: return "Left";
        case sf::Keyboard::Right: return "Right";
        
        // Модификаторы
        case sf::Keyboard::LShift: return "LShift";
        case sf::Keyboard::RShift: return "RShift";
        case sf::Keyboard::LControl: return "LControl";
        case sf::Keyboard::RControl: return "RControl";
        case sf::Keyboard::LAlt: return "LAlt";
        case sf::Keyboard::RAlt: return "RAlt";
        case sf::Keyboard::LSystem: return "LSystem";
        case sf::Keyboard::RSystem: return "RSystem";
        
        default: return "Unknown";
    }
}

void GUIInputHandler::PrintKeyBindings() const {
    std::cout << "\n=== Текущие назначения клавиш (GUI) ===\n";
    std::string scheme_name = (movement_scheme_ == MovementScheme::WASD) ? "WASD" : "СТРЕЛКИ";
    std::cout << "Схема движения: " << scheme_name << "\n\n";

    for (const auto& [key, command] : key_bindings_) {
        std::cout << KeyToString(key) << " -> " << CommandToString(command) << "\n";
    }
    std::cout << "=======================================\n\n";
}


void GUIInputHandler::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        auto it = key_bindings_.find(event.key.code);
        if (it != key_bindings_.end()) {
            std::unique_ptr<Command> command = std::make_unique<Command>(it->second);
            std::cout << "Обработано событие: " << CommandToString(it->second) << std::endl;
        }
    }
}


const std::map<sf::Keyboard::Key, CommandType>& GUIInputHandler::key_bindings() const { 
    return key_bindings_; 
}

const std::string GUIInputHandler::control_legend() {
    std::string move_key = (movement_scheme_ == MovementScheme::WASD) ? "WASD" : "СТРЕЛКИ";
    std::string attack_key, ability_key, save_key, load_key, pause_key, place_ship_key, rotate_key,
                remove_key, show_ships_key, restart_key, help_key, stats_key,
                field_key, ship_size_key, toggle_placement_key, exit_key, yes_key, no_key,
                set_1_key, set_2_key, set_3_key, set_4_key, set_5_key;

    for (const auto& [key, command] : key_bindings_) {
        std::string keyName = KeyToString(key);

        switch (command) {
            case CommandType::ATTACK:               attack_key = keyName; break;
            case CommandType::USE_ABILITY:          ability_key = keyName; break;
            case CommandType::SAVE:                 save_key = keyName; break;
            case CommandType::LOAD:                 load_key = keyName; break;
            case CommandType::PAUSE:                pause_key = keyName; break;
            case CommandType::PLACE_SHIP:           place_ship_key = keyName; break;
            case CommandType::ROTATE_SHIP:          rotate_key = keyName; break;
            case CommandType::REMOVE_SHIP:          remove_key = keyName; break;
            case CommandType::SHOW_SHIPS:           show_ships_key = keyName; break;
            case CommandType::RESTART:              restart_key = keyName; break;
            case CommandType::HELP:                 help_key = keyName; break;
            case CommandType::STATS:                stats_key = keyName; break;
            case CommandType::SET_NEW_FIELD:        field_key = keyName; break;
            case CommandType::SET_NEW_SHIP_SIZES:   ship_size_key = keyName; break;
            case CommandType::TOGGLE_PLACEMENT_MODE:toggle_placement_key = keyName; break;
            case CommandType::EXIT:                 exit_key = keyName; break;
            case CommandType::YES:                  yes_key = keyName; break;
            case CommandType::NO:                   no_key = keyName; break;
            case CommandType::SET_1:                set_1_key = keyName; break;
            case CommandType::SET_2:                set_2_key = keyName; break;
            case CommandType::SET_3:                set_3_key = keyName; break;
            case CommandType::SET_4:                set_4_key = keyName; break;
            case CommandType::SET_5:                set_5_key = keyName; break;
            default: break;
        }
    }

    if (attack_key.empty())          attack_key = "Space";
    if (ability_key.empty())         ability_key = "U";
    if (save_key.empty())            save_key = "F2";
    if (restart_key.empty())         restart_key = "F5";
    if (load_key.empty())            load_key = "L";
    if (pause_key.empty())           pause_key = "ESC";
    if (place_ship_key.empty())      place_ship_key = "P";
    if (rotate_key.empty())          rotate_key = "R";
    if (remove_key.empty())          remove_key = "D";
    if (show_ships_key.empty())      show_ships_key = "S";
    if (help_key.empty())            help_key = "H";
    if (stats_key.empty())           stats_key = "T";
    if (field_key.empty())           field_key = "E";
    if (ship_size_key.empty())       ship_size_key = "Z";
    if (toggle_placement_key.empty())toggle_placement_key = "A";
    if (exit_key.empty())            exit_key = "Q";
    if (yes_key.empty())             yes_key = "Y";
    if (no_key.empty())              no_key = "N";
    if (set_1_key.empty())           set_1_key = "1";
    if (set_2_key.empty())           set_2_key = "2";
    if (set_3_key.empty())           set_3_key = "3";
    if (set_4_key.empty())           set_4_key = "4";
    if (set_5_key.empty())           set_5_key = "5";

    return " ОСНОВНОЕ: [" + attack_key + "] - выстрел | [" + ability_key + "] - способность | [" + move_key + "] - курсор\n"
           " КОРАБЛИ: [" + place_ship_key + "] - разместить | [" + rotate_key + "] - повернуть | [" + remove_key + "] - удалить | ["
           + show_ships_key + "] - показать\n"
           " ДОП: [" + field_key + "] - изменить поле | [" + ship_size_key + "] - изменить корабли | [" + toggle_placement_key
           + "] - переключить режим расстановки\n"
           " ВЫБОР: [" + set_1_key + "] - выб_1 | [" + set_2_key + "] - выб_2 | [" + set_3_key + "] - выб_3 | [" + set_4_key
           + "] - выб_4 | [" + set_5_key + "] - выб_5 | [" + yes_key + "]/[" + no_key + "] - ДА/НЕТ\n"
           " СИСТЕМА: [" + save_key + "]/[" + load_key + "] - сохр/загр | [" + pause_key + "] - пауза | ["
           + restart_key + "] - перезапуск | [" + stats_key + "] - статистика | [" + help_key + "] - помощь | ["
           + exit_key + "] - выход\n";
}