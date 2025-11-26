#include "TerminalInputHandler.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <set>



TerminalInputHandler::TerminalInputHandler(const std::string& configFile)
    : config_file_(configFile), movement_scheme_(MovementScheme::WASD) {}

TerminalInputHandler::~TerminalInputHandler() {
    RestoreTerminal();
}

bool TerminalInputHandler::Initialize() {
    if (!SetUpTerminal()) {
        std::cerr << "Ошибка настройки терминала!\n";
        return false;
    }

    if (!loadFromFile()) {
        std::cout << "Не удалось загрузить настройки из файла '" << config_file_
                  << "'. Используются настройки по умолчанию.\n";
        SetDefaultBindings();  // ← только безопасные клавиши
    }

    // Сначала — исправляем недостающие команды (W/A/S/D ещё НЕ заняты!)
    if (!ValidateOrFixBindings()) {
        std::cout << "Некоторые назначения клавиш были автоматически исправлены.\n";
        SaveDefaultConfig();
    }

    // ← Только ПОСЛЕ этого — назначаем движение (оно "побеждает")
    SetMovementBindings();

    std::cout << "Управление успешно настроено. Загружено "
              << key_bindings_.size() << " назначений клавиш.\n";
    PrintKeyBindings();
    return true;
}


bool TerminalInputHandler::SetUpTerminal() {
    if (tcgetattr(STDIN_FILENO, &original_termios_) == -1) {
        return false;
    }
    struct termios new_termios = original_termios_;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN]  = 0; // неблокирующий
    new_termios.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) != -1;
}

void TerminalInputHandler::RestoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios_);
}


void TerminalInputHandler::SetDefaultBindings() {
    key_bindings_.clear();

    key_bindings_[' '] = CommandType::ATTACK;
    key_bindings_['p'] = CommandType::PLACE_SHIP;
    key_bindings_['r'] = CommandType::ROTATE_SHIP;
    key_bindings_['q'] = CommandType::EXIT;
    key_bindings_['u'] = CommandType::USE_ABILITY;
    key_bindings_['h'] = CommandType::HELP;
    key_bindings_['t'] = CommandType::STATS;
    key_bindings_['l'] = CommandType::LOAD;
    key_bindings_['k'] = CommandType::SAVE;
    key_bindings_['f'] = CommandType::RESTART;
    key_bindings_[27]  = CommandType::PAUSE;
    key_bindings_['x'] = CommandType::SET_NEW_FIELD;
    key_bindings_['z'] = CommandType::SET_NEW_SHIP_SIZES;
    key_bindings_['1'] = CommandType::SET_1;
    key_bindings_['2'] = CommandType::SET_2;
    key_bindings_['3'] = CommandType::SET_3;
    key_bindings_['4'] = CommandType::SET_4;
    key_bindings_['5'] = CommandType::SET_5;
    key_bindings_['y'] = CommandType::YES;
    key_bindings_['n'] = CommandType::NO;

    if (movement_scheme_ == MovementScheme::WASD) {
        key_bindings_['i'] = CommandType::SHOW_SHIPS;
        key_bindings_['e'] = CommandType::REMOVE_SHIP;
        key_bindings_['o'] = CommandType::TOGGLE_PLACEMENT_MODE;
    } else {
        key_bindings_['s'] = CommandType::SHOW_SHIPS;
        key_bindings_['d'] = CommandType::REMOVE_SHIP;
        key_bindings_['a'] = CommandType::TOGGLE_PLACEMENT_MODE;
    }
}

bool TerminalInputHandler::ValidateOrFixBindings() {
    bool fixed = false;
    std::set<char> occupied;
    for (const auto& p : key_bindings_) occupied.insert(p.first);

    const std::vector<char> move_keys = (movement_scheme_ == MovementScheme::WASD)
        ? std::vector<char>{'w','a','s','d'} : std::vector<char>{};

    auto is_reserved = [&](char c) -> bool {
        if (movement_scheme_ == MovementScheme::WASD) {
            char lc = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return lc == 'w' || lc == 'a' || lc == 's' || lc == 'd';
        }
        return false;  // в ARROWS-схеме w/a/s/d свободны для назначения
    };

    auto free_digit = [&]() -> char {
        for (char c = '0'; c <= '9'; ++c)
            if (!occupied.count(c) && !is_reserved(c)) return c;
        return 0;
    };

    auto free_letter = [&]() -> char {
        for (char c = 'a'; c <= 'z'; ++c)
            if (!occupied.count(c) && !is_reserved(c)) return c;
        return 0;
    };

    struct Req { CommandType cmd; char pref; };
    const std::vector<Req> required = {
        {CommandType::ATTACK,              ' '},
        {CommandType::PLACE_SHIP,          'p'},
        {CommandType::ROTATE_SHIP,         'r'},
        {CommandType::EXIT,                'q'},
        {CommandType::USE_ABILITY,         'u'},
        {CommandType::HELP,                'h'},
        {CommandType::STATS,               't'},
        {CommandType::LOAD,                'l'},
        {CommandType::SAVE,                'k'},
        {CommandType::RESTART,             'f'},
        {CommandType::PAUSE,               27},
        {CommandType::SET_NEW_FIELD,       'x'},
        {CommandType::SET_NEW_SHIP_SIZES,  'z'},
        {CommandType::YES,                 'y'},
        {CommandType::NO,                  'n'},
        {CommandType::SET_1,               '1'},
        {CommandType::SET_2,               '2'},
        {CommandType::SET_3,               '3'},
        {CommandType::SET_4,               '4'},
        {CommandType::SET_5,               '5'},
        {CommandType::SHOW_SHIPS,          movement_scheme_ == MovementScheme::WASD ? 'i' : 's'},
        {CommandType::REMOVE_SHIP,         movement_scheme_ == MovementScheme::WASD ? 'e' : 'd'},
        {CommandType::TOGGLE_PLACEMENT_MODE, movement_scheme_ == MovementScheme::WASD ? 'o' : 'a'}
    };

    for (const auto& r : required) {
        bool exists = false;
        for (const auto& p : key_bindings_) if (p.second == r.cmd) { exists = true; break; }
        if (exists) continue;

        char key = r.pref;
        if (occupied.count(key) || is_reserved(key)) {
            if (r.cmd >= CommandType::SET_1 && r.cmd <= CommandType::SET_5) {
                key = free_digit();
                if (!key) key = free_letter();
            } else {
                key = free_letter();
                if (!key) key = free_digit();
            }
        }
        if (!key) {
            std::cerr << "Не удалось найти свободную клавишу для " << CommandToString(r.cmd) << "\n";
            return false;
        }

        key_bindings_[key] = r.cmd;
        occupied.insert(key);
        fixed = true;
        std::cout << "Автоназначение: " << CommandToString(r.cmd) << " → "
                  << KeyToDisplayString(key) << "\n";
    }
    return !fixed;
}


void TerminalInputHandler::SetMovementBindings() {
    // Удаляем старые команды движения, если были
    std::vector<char> to_remove;
    for (const auto& p : key_bindings_) {
        if (p.second >= CommandType::MOVE_UP && p.second <= CommandType::MOVE_RIGHT) {
            to_remove.push_back(p.first);
        }
    }
    for (char k : to_remove) key_bindings_.erase(k);

    if (movement_scheme_ == MovementScheme::WASD) {
        key_bindings_['w'] = CommandType::MOVE_UP;
        key_bindings_['s'] = CommandType::MOVE_DOWN;
        key_bindings_['a'] = CommandType::MOVE_LEFT;
        key_bindings_['d'] = CommandType::MOVE_RIGHT;
    }
    // Стрелки обрабатываются отдельно через ESC-последовательности — их в key_bindings_ не кладём
}

CommandType TerminalInputHandler::ParseCommand(const std::string& command_str) const {
    std::string str_lower = ToLower(command_str);
    if (str_lower == "move_up")     return CommandType::MOVE_UP;
    if (str_lower == "move_down")   return CommandType::MOVE_DOWN;
    if (str_lower == "move_left")   return CommandType::MOVE_LEFT;
    if (str_lower == "move_right")  return CommandType::MOVE_RIGHT;
    if (str_lower == "attack")      return CommandType::ATTACK;
    if (str_lower == "exit")        return CommandType::EXIT;
    if (str_lower == "restart")     return CommandType::RESTART;
    if (str_lower == "help")        return CommandType::HELP;
    if (str_lower == "show_ships")     return CommandType::SHOW_SHIPS;
    if (str_lower == "use_ability") return CommandType::USE_ABILITY;
    if (str_lower == "stats")       return CommandType::STATS;
    if (str_lower == "load")        return CommandType::LOAD;
    if (str_lower == "save")        return CommandType::SAVE;
    if (str_lower == "pause")       return CommandType::PAUSE;
    if (str_lower == "remove_ship") return CommandType::REMOVE_SHIP;
    if (str_lower == "place_ship")  return CommandType::PLACE_SHIP;
    if (str_lower == "rotate_ship") return CommandType::ROTATE_SHIP;
    if (str_lower == "set_new_field")       return CommandType::SET_NEW_FIELD;
    if (str_lower == "set_new_ship_sizes")  return CommandType::SET_NEW_SHIP_SIZES;
    if (str_lower == "set_1")               return CommandType::SET_1;
    if (str_lower == "set_2")               return CommandType::SET_2;
    if (str_lower == "set_3")               return CommandType::SET_3;
    if (str_lower == "set_4")               return CommandType::SET_4;
    if (str_lower == "set_5")               return CommandType::SET_5;
    if (str_lower == "yes")               return CommandType::YES;
    if (str_lower == "no")               return CommandType::NO;
    if (str_lower == "toggle_placement_mode")    return CommandType::TOGGLE_PLACEMENT_MODE;
    return CommandType::UNKNOWN;
}

std::string TerminalInputHandler::CommandToString(CommandType command) const {
    switch (command) {
        case CommandType::MOVE_UP:     return "MOVE_UP";
        case CommandType::MOVE_DOWN:   return "MOVE_DOWN";
        case CommandType::MOVE_LEFT:   return "MOVE_LEFT";
        case CommandType::MOVE_RIGHT:  return "MOVE_RIGHT";
        case CommandType::ATTACK:      return "ATTACK";
        case CommandType::EXIT:        return "EXIT";
        case CommandType::RESTART:     return "RESTART";
        case CommandType::HELP:        return "HELP";
        case CommandType::USE_ABILITY: return "USE_ABILITY";
        case CommandType::STATS:       return "STATS";
        case CommandType::LOAD:        return "LOAD";
        case CommandType::SAVE:        return "SAVE";
        case CommandType::PAUSE:       return "PAUSE";
        case CommandType::SHOW_SHIPS:  return "SHOW_SHIPS";
        case CommandType::REMOVE_SHIP: return "REMOVE_SHIP";
        case CommandType::PLACE_SHIP:  return "PLACE_SHIP";
        case CommandType::ROTATE_SHIP: return "ROTATE_SHIP";
        case CommandType::SET_NEW_FIELD:        return "SET_NEW_FIELD";
        case CommandType::SET_NEW_SHIP_SIZES:   return "SET_NEW_SHIP_SIZES";
        case CommandType::SET_1:                return "SET_1";
        case CommandType::SET_2:                return "SET_2";
        case CommandType::SET_3:                return "SET_3";
        case CommandType::SET_4:                return "SET_4";
        case CommandType::SET_5:                return "SET_5";
        case CommandType::YES:                return "YES";
        case CommandType::NO:                return "NO";
        case CommandType::TOGGLE_PLACEMENT_MODE:   return "TOGGLE_PLACEMENT_MODE";
        default:                       return "UNKNOWN";
    }
}

const std::map<char, CommandType>& TerminalInputHandler::key_bindings() const { 
    return key_bindings_; 
}

const std::string TerminalInputHandler::control_legend() {
    std::string move_key = (movement_scheme_ == MovementScheme::WASD) ? "WASD" : "СТРЕЛКИ";
    std::string attack_key, ability_key, save_key, load_key, pause_key, place_ship_key, rotate_key,
                remove_key, show_ships_key, restart_key, help_key, stats_key,
                field_key, ship_size_key, toggle_placement_key, exit_key, yes_key, no_key,
                set_1_key, set_2_key, set_3_key, set_4_key, set_5_key;

    for (const auto& [key, command] : key_bindings_) {
        std::string key_name = (key == ' ')  ? "SPACE" :
                              (key == 27)   ? "ESC"   : std::string(1, std::toupper(key));

        switch (command) {
            case CommandType::ATTACK:               attack_key = key_name; break;
            case CommandType::USE_ABILITY:          ability_key = key_name; break;
            case CommandType::SAVE:                 save_key = key_name; break;
            case CommandType::LOAD:                 load_key = key_name; break;
            case CommandType::PAUSE:                pause_key = key_name; break;
            case CommandType::PLACE_SHIP:           place_ship_key = key_name; break;
            case CommandType::ROTATE_SHIP:          rotate_key = key_name; break;
            case CommandType::REMOVE_SHIP:          remove_key = key_name; break;
            case CommandType::SHOW_SHIPS:           show_ships_key = key_name; break;
            case CommandType::RESTART:              restart_key = key_name; break;
            case CommandType::HELP:                 help_key = key_name; break;
            case CommandType::STATS:                stats_key = key_name; break;
            case CommandType::SET_NEW_FIELD:        field_key = key_name; break;
            case CommandType::SET_NEW_SHIP_SIZES:   ship_size_key = key_name; break;
            case CommandType::TOGGLE_PLACEMENT_MODE:   toggle_placement_key = key_name; break;
            case CommandType::EXIT:                 exit_key = key_name; break;
            case CommandType::YES:                  yes_key = key_name; break;
            case CommandType::NO:                   no_key = key_name; break;
            case CommandType::SET_1:                set_1_key = key_name; break;
            case CommandType::SET_2:                set_2_key = key_name; break;
            case CommandType::SET_3:                set_3_key = key_name; break;
            case CommandType::SET_4:                set_4_key = key_name; break;
            case CommandType::SET_5:                set_5_key = key_name; break;
            default: break;
        }
    }

    if (attack_key.empty())           attack_key = "SPACE";
    if (ability_key.empty())          ability_key = "U";
    if (save_key.empty())             save_key = "O";
    if (restart_key.empty())          restart_key = "F";
    if (load_key.empty())              load_key = "L";
    if (pause_key.empty())            pause_key = "ESC";
    if (place_ship_key.empty())       place_ship_key = "P";
    if (rotate_key.empty())           rotate_key = "R";
    if (remove_key.empty())           remove_key = "E";
    if (show_ships_key.empty())       show_ships_key = "I";
    if (help_key.empty())             help_key = "H";
    if (stats_key.empty())            stats_key = "T";
    if (field_key.empty())            field_key = "N";
    if (ship_size_key.empty())        ship_size_key = "M";
    if (toggle_placement_key.empty()) toggle_placement_key = "X";
    if (exit_key.empty())             exit_key = "Q";
    if (yes_key.empty())              yes_key = "K";
    if (no_key.empty())             no_key = "B";
    if (set_1_key.empty())            set_1_key = "1";
    if (set_2_key.empty())            set_2_key = "2";
    if (set_3_key.empty())            set_3_key = "3";
    if (set_4_key.empty())            set_4_key = "4";
    if (set_5_key.empty())            set_5_key = "5";

    return "ОСНОВНОЕ: [" + attack_key + "] Выстрел | [" + ability_key + "] Способность | [" + move_key + "] Курсор\n"
           "КОРАБЛИ: [" + place_ship_key + "] Разместить | [" + rotate_key + "] Повернуть | [" + remove_key + "] Удалить | ["
           + show_ships_key + "] Показать\n"
           "ДОП: [" + field_key + "]/[" + ship_size_key + "] Изменить поле/корабли | [" + toggle_placement_key
           + "] Другой режим расстановки | [" + stats_key + "] Статистика\n"
           "ВЫБОР: [" + set_1_key + "] Выб_1 | [" + set_2_key + "] Выб_2 | [" + set_3_key + "] Выб_3 | [" + set_4_key
           + "] Выб_4 | [" + set_5_key + "] Выб_5 | [" + yes_key + "]/[" + no_key + "] Да/Нет\n"
           "СИСТЕМА: [" + save_key + "]/[" + load_key + "] Сохр/Загр | [" + pause_key + "] Пауза | ["
           + restart_key + "] Перезапуск | [" + help_key + "] Помощь | [" + exit_key + "] Выход";
}



bool TerminalInputHandler::loadFromFile() {
    std::ifstream file(config_file_);
    if (!file.is_open()) {
        return false;
    }

    key_bindings_.clear();
    movement_scheme_ = MovementScheme::WASD;

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
                std::string scheme = ToLower(line.substr(pos + 1));
                Trim(scheme);
                if (scheme == "wasd") movement_scheme_ = MovementScheme::WASD;
                else if (scheme == "arrows" || scheme == "arrow") movement_scheme_ = MovementScheme::ARROWS;
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

        char original_key = ParseTerminalKey(key_str);
        if (original_key == 0) {
            std::cerr << "Ошибка в строке " << line_number << ": неизвестная клавиша '" << key_str << "'\n";
            continue;
        }

        // === Проверка: не попадает ли клавиша под движение (WASD) ===
        bool conflict_with_movement = false;
        if (movement_scheme_ == MovementScheme::WASD) {
            char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(original_key)));
            if (lower == 'w' || lower == 'a' || lower == 's' || lower == 'd') {
                conflict_with_movement = true;
            }
        }

        if (conflict_with_movement) {
            std::cout << "Конфликт в строке " << line_number
                      << ": клавиша '" << KeyToDisplayString(original_key)
                      << "' зарезервирована под движение. Команда '" << cmd_str
                      << "' будет переназначена автоматически.\n";

            // Поиск свободной и безопасной клавиши ===
            char new_key = 0;

            // Функция: свободна ли клавиша (и не зарезервирована под движение)
            auto is_safe_and_free = [&](char c) -> bool {
                // Проверка на занятость в key_bindings_
                if (key_bindings_.find(c) != key_bindings_.end()) return false;
                if (key_bindings_.find(static_cast<char>(std::toupper(c))) != key_bindings_.end()) return false;
                if (key_bindings_.find(static_cast<char>(std::tolower(c))) != key_bindings_.end()) return false;

                // Проверка на зарезервированность под WASD
                if (movement_scheme_ == MovementScheme::WASD) {
                    char lc = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    if (lc == 'w' || lc == 'a' || lc == 's' || lc == 'd') return false;
                }
                return true;
            };

            // Поиск с приоритетами
            if (cmd >= CommandType::SET_1 && cmd <= CommandType::SET_5) {
                for (char c = '0'; c <= '9'; ++c)
                    if (is_safe_and_free(c)) { new_key = c; break; }
                if (!new_key)
                    for (char c = 'a'; c <= 'z'; ++c)
                        if (is_safe_and_free(c)) { new_key = c; break; }
            } else {
                for (char c = 'a'; c <= 'z'; ++c)
                    if (is_safe_and_free(c)) { new_key = c; break; }
                if (!new_key)
                    for (char c = '0'; c <= '9'; ++c)
                        if (is_safe_and_free(c)) { new_key = c; break; }
            }

            if (new_key != 0) {
                key_bindings_[new_key] = cmd;
                std::cout << "→ '" << cmd_str << "' переназначена на '" << KeyToDisplayString(new_key) << "'\n";
                had_conflict = true;
            } else {
                std::cerr << "Критическая ошибка: не удалось найти безопасную клавишу для '" << cmd_str << "'\n";
            }
        } else {
            // Нет конфликта с движением — просто назначаем
            if (key_bindings_.find(original_key) != key_bindings_.end()) {
                std::cout << "Предупреждение в строке " << line_number
                          << ": клавиша '" << KeyToDisplayString(original_key) << "' перезаписана\n";
            }
            key_bindings_[original_key] = cmd;
        }
    }

    file.close();

    if (had_conflict) {
        std::cout << "Настройки загружены с автоматическим исправлением конфликтов.\n";
        SaveDefaultConfig(); 
    }

    return true; 
}


std::unique_ptr<Command> TerminalInputHandler::CreateMoveCommand(CommandType command) {
    auto now = std::chrono::steady_clock::now();
    if (now - last_move_ < std::chrono::milliseconds(repeat_ms_))
        return nullptr;
    last_move_ = now;
    return std::make_unique<Command>(command);
}


char TerminalInputHandler::ParseTerminalKey(const std::string& key_str) const {
    std::string lower_key = ToLower(key_str);

    if (lower_key.length() == 1) {
        char c = lower_key[0];
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            return c;
        }
        switch (c) {
            case '!': case '@': case '#': case '$': case '%': case '^': case '&': case '*': 
            case '(': case ')': case '_': case '+': case '-': case '=': case '[': case ']':
            case '{': case '}': case '|': case '\\': case ';': case ':': case '\'': case '"':
            case ',': case '.': case '<': case '>': case '/': case '?': case '`': case '~':
                return c;
        }
    }

    if (lower_key == "space" || lower_key == "spc") return ' ';
    if (lower_key == "escape" || lower_key == "esc") return 27;
    if (lower_key == "enter" || lower_key == "return") return '\n';
    if (lower_key == "tab") return '\t';
    if (lower_key == "backspace") return 127;
    
    for (int i = 1; i <= 12; i++) {
        if (lower_key == "f" + std::to_string(i)) {
            return static_cast<char>(i);
        }
    }

    return 0; 
}

std::string TerminalInputHandler::KeyToDisplayString(char key) const {
    if (key == ' ') return "SPACE";
    if (key == 27) return "ESC";
    if (key == '\n') return "ENTER";
    if (key == '\t') return "TAB";
    if (key == 127) return "BACKSPACE";
    
    if (key >= 1 && key <= 12) {
        return "F" + std::to_string(static_cast<int>(key));
    }
    
    return std::string(1, key);
}

bool TerminalInputHandler::SaveDefaultConfig() const {
    std::ofstream file(config_file_);
    if (!file.is_open()) return false;

    file << "# КОНФИГУРАЦИЯ УПРАВЛЕНИЯ - МОРСКОЙ БОЙ (КОНСОЛЬ)\n";
    file << "# Формат: клавиша = команда\n";
    file << "# Регистр клавиш не важен. Не назначайте разные команды на одну клавишу.\n\n";

    file << "# Выбор схемы управления (WASD или ARROWS)\n";
    file << "# В режиме WASD: W,A,S,D = движение, Стрелки = не назначены\n";
    file << "# В режиме ARROWS: Стрелки = движение, W,A,S,D = не назначены\n";
    std::string current_scheme = (movement_scheme_ == MovementScheme::WASD) ? "WASD" : "ARROWS";
    file << "movement_scheme = " << current_scheme << "\n\n";

    file << "# Основные команды:\n";
    file << "# Вы можете изменить клавиши, но не меняйте названия команд после '='\n\n";

    for (const auto& [key, command] : key_bindings_) {
        file << KeyToDisplayString(key) << " = " << CommandToString(command) << "\n";
    }

    file << "\n# ПОДСКАЗКИ:\n";
    file << "# - После изменения файла перезапустите игру\n";
    file << "# - Для сброса настроек удалите этот файл\n";
    file << "# - Команды движения настраиваются через movement_scheme\n";
   
    file.close();
    return true;
}

std::unique_ptr<Command> TerminalInputHandler::GetCommand() {
    char input[8] = {0};
    ssize_t n = read(STDIN_FILENO, input, 1);
    
    if (n == 1) {
        if (input[0] == 27) {
            // Обработка ESC и стрелок
            struct timeval tv = {0, 5000};
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            
            if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
                ssize_t n2 = read(STDIN_FILENO, input + 1, 7);
                if (n2 > 0 && input[1] == '[') {
                    // Обработка стрелок
                    if (movement_scheme_ == MovementScheme::ARROWS) {
                        CommandType arrow_command = CommandType::UNKNOWN;
                        switch (input[2]) {
                            case 'A': arrow_command = CommandType::MOVE_UP; break;
                            case 'B': arrow_command = CommandType::MOVE_DOWN; break;
                            case 'C': arrow_command = CommandType::MOVE_RIGHT; break;
                            case 'D': arrow_command = CommandType::MOVE_LEFT; break;
                            default: break;
                        }
                        
                        if (arrow_command != CommandType::UNKNOWN) {
                            return CreateMoveCommand(arrow_command);
                        }
                    }
                } else {
                    // Обработка функциональных клавиш F1-F12
                    if (n2 == 1) {
                        char func_key = input[1];
                        if (func_key >= 'P' && func_key <= 'S') { // F1-F4
                            int f_num = func_key - 'P' + 1;
                            auto it = key_bindings_.find(static_cast<char>(f_num));
                            if (it != key_bindings_.end()) {
                                return std::make_unique<Command>(it->second);
                            }
                        }
                    }
                }
            }
            // ESC без последующих символов = PAUSE
            return std::make_unique<Command>(CommandType::PAUSE);
        }
        
        // Обычные клавиши
        char key = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
        
        // Обработка WASD движения (только в WASD схеме)
        if (movement_scheme_ == MovementScheme::WASD) {
            CommandType wasd_command = CommandType::UNKNOWN;
            switch (key) {
                case 'w': wasd_command = CommandType::MOVE_UP; break;
                case 's': wasd_command = CommandType::MOVE_DOWN; break;
                case 'a': wasd_command = CommandType::MOVE_LEFT; break;
                case 'd': wasd_command = CommandType::MOVE_RIGHT; break;
                default: break;
            }
            
            if (wasd_command != CommandType::UNKNOWN) {
                return CreateMoveCommand(wasd_command);
            }
        }
        
        // Остальные клавиши из конфига
        auto it = key_bindings_.find(key);
        if (it != key_bindings_.end()) {
            return std::make_unique<Command>(it->second);
        }
    }
    
    return nullptr;
}

void TerminalInputHandler::PrintKeyBindings() const {
    std::cout << "\n=== Текущие назначения клавиш (консоль) ===\n";

    std::string scheme_name = (movement_scheme_ == MovementScheme::WASD) ? "WASD" : "СТРЕЛКИ";
    std::cout << "Схема движения: " << scheme_name << "\n\n";

    for (const auto& [key, command] : key_bindings_) {
        std::cout << KeyToDisplayString(key) << " -> " << CommandToString(command) << "\n";
    }
    std::cout << "===========================================\n\n";
}