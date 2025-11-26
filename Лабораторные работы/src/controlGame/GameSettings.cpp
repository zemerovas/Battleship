#include "GameSettings.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>

GameSettings::GameSettings(): interface_type_(InterfaceType::GUI), field_size_(10),
      cell_size_(40), player_name_("Player"), fleet_mode_(FleetBuildMode::STANDARD),
      placement_mode_(PlacementMode::AUTO), fleet_spec_{4,3,3,2,2,2,1,1,1,1} {}

      
static int readIntOrDefaultWithWarn(int default_value, int min_value, int max_value) {
    int user_input;
    if (std::cin >> user_input) {
        if (user_input < min_value || user_input > max_value) {
            std::cout << "Некорректный ввод (" << user_input << "). Будет применено значение по умолчанию (" << default_value << ").\n";
            return default_value;
        }
        return user_input;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Некорректный ввод. Будет применено значение по умолчанию (" << default_value << ").\n";
    return default_value;
}


static std::vector<int> parseFleetLine(const std::string& line) {
    std::vector<int> out;
    std::stringstream ss(line);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        if (tok.empty()) continue;
        try {
            int x = std::stoi(tok);
            if (x >= 1 && x <= 4) out.push_back(x);
        } catch (...) {}
    }
    return out;
}


void GameSettings::ShowSettingsDialog() {
    std::cout << "=== НАСТРОЙКИ МОРСКОГО БОЯ ===\n";
    std::cout << "Выберите тип интерфейса:\n"
                 "1. Графический интерфейс (GUI)\n"
                 "2. Консольный интерфейс\n"
                 "Ваш выбор (1-2): ";
    int choice = readIntOrDefaultWithWarn(1, 1, 2);
    interface_type_ = (choice == 2) ? InterfaceType::CONSOLE : InterfaceType::GUI;

    if (interface_type_ == InterfaceType::GUI) {
        std::cout << "\nВыберите размер клетки:\n"
                     "1. Маленький (30px)\n"
                     "2. Средний (40px)\n"
                     "3. Большой (50px)\n"
                     "- при некорректном выборе будет 40px\n"
                     "Ваш выбор (1-3): ";
        int csz = readIntOrDefaultWithWarn(2, 1, 3);
        cell_size_ = (csz == 1) ? 30 : (csz == 3) ? 50 : 40;
    }
    
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (true) {
        std::cout << "Введите ваше имя (макс 10 символов): ";
        std::getline(std::cin, player_name_);

        size_t start = player_name_.find_first_not_of(" \t\n\r");
        size_t end   = player_name_.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) {
            player_name_.clear();
        } else {
            player_name_ = player_name_.substr(start, end - start + 1);
        }

        size_t count = 0;
        size_t i = 0;
        for (; i < player_name_.size() && count < 10; ++i) {
            unsigned char c = player_name_[i];
            // Начало нового UTF-8 символа
            if ((c & 0xC0) != 0x80) ++count;
        }
        player_name_ = player_name_.substr(0, i);

        // Проверка: есть хотя бы один буква/цифра или кириллица
        bool has_letter_or_digit = false;
        for (size_t j = 0; j < player_name_.size(); ) {
            unsigned char c = player_name_[j];
            size_t char_len = 1;

            if ((c & 0x80) != 0) { // UTF-8 многобайтовый
                if ((c & 0xE0) == 0xC0) char_len = 2;
                else if ((c & 0xF0) == 0xE0) char_len = 3;
                else if ((c & 0xF8) == 0xF0) char_len = 4;
            }

            if (char_len == 1 && std::isalnum(c)) {
                has_letter_or_digit = true;
                break;
            } else if (char_len > 1) { // кириллица и другие символы >1 байта
                has_letter_or_digit = true;
                break;
            }

            j += char_len;
        }

        if (!player_name_.empty() && has_letter_or_digit)
            break;

        std::cout << "Имя некорректное (пустое или нет букв/цифр), попробуйте снова.\n";
    }

    ConfigureGameSettings();
}


void GameSettings::ConfigureGameSettings(){
    std::cout << "\nОпределение размеров игрового поля:\n"
                    "Ваш выбор (10-14): ";
    field_size_ = readIntOrDefaultWithWarn(10, 10, 14);
    
    std::cout << "\nСпособ формирования флота:\n"
                 "1. Стандартный (1x4, 2x3, 3x2, 4x1)\n"
                 "2. Свой список размеров\n"
                 "Ваш выбор (1-2): ";
    int temp_fl_mode = readIntOrDefaultWithWarn(1, 1, 2);
    fleet_mode_ = (temp_fl_mode == 2) ? FleetBuildMode::CUSTOM : FleetBuildMode::STANDARD;

    if (fleet_mode_ == FleetBuildMode::CUSTOM) {
        std::cout << "Введите размеры кораблей (максимально 16 кораблей) через запятую (пример: 4,3,3,2,2,2,1,1,1,1): ";
        std::string line;
        std::cin >> line;
        fleet_spec_ = parseFleetLine(line);
        if (fleet_spec_.size() > 16 || fleet_spec_.empty()) {
            std::cout << "Некорректный ввод. Применён стандартный набор.\n";
            fleet_mode_ = FleetBuildMode::STANDARD;
        }
    }

    std::cout << "\nРасстановка кораблей:\n"
                 "1. Автоматическая\n"
                 "2. Ручная\n"
                 "Ваш выбор (1-2): ";
    int temp_pl_mode = readIntOrDefaultWithWarn(1, 1, 2);
    placement_mode_ = (temp_pl_mode == 2) ? PlacementMode::MANUAL : PlacementMode::AUTO;

    std::cout << "\nНастройки сохранены!\n";
}


void GameSettings::AddShipSize(int size){
    if (temp_fleet_spec_.size() < 16){
        temp_fleet_spec_.push_back(size);
    }
}


void GameSettings::ClearTempFleetSpec(){
    temp_fleet_spec_.clear();
}


void GameSettings::ApplyFleetSpec(){
    fleet_spec_ = temp_fleet_spec_;
}


void GameSettings::ApplyFieldSize(){
    field_size_ = temp_field_size_;
}


void GameSettings::ResetFieldAndShipSize(){
    field_size_ = 10;
    set_fleet_mode();
}


void GameSettings::set_fleet_mode(bool custom){
    if (custom) fleet_mode_ = FleetBuildMode::CUSTOM;
    else {
        fleet_mode_ = FleetBuildMode::STANDARD;
        fleet_spec_ = {4,3,3,2,2,2,1,1,1,1};
    }
}


int GameSettings::temp_field_size() const { 
    return temp_field_size_; 
}


void GameSettings::set_temp_field_size(int size){
    if (interface_type_ == InterfaceType::GUI){
        if (cell_size_ == 40 && size == 14){
            return;
        }
        if (cell_size_ == 50){
            if (size != 10){
                return ;
            }
        }
    }
    
    temp_field_size_ = size;
}


InterfaceType GameSettings::interface_type() const { 
    return interface_type_; 
}


int GameSettings::cell_size() const { 
    return cell_size_; 
}


int GameSettings::field_size() const { 
    return field_size_; 
}


void GameSettings::set_field_size(int size) { 
    field_size_ = size; 
}  


PlacementMode GameSettings::placement_mode() const { 
    return placement_mode_; 
} 


void GameSettings::set_placement_mode(PlacementMode mode) { 
    placement_mode_ = mode; 
}

const std::string& GameSettings::player_name() const { 
    return player_name_; 
}


void GameSettings::set_player_name(const std::string& name) { 
    player_name_ = name; 
}

const std::vector<int>& GameSettings::fleet_spec() const { 
    return fleet_spec_; 
}


const std::vector<int>& GameSettings::temp_fleet_spec() const { 
    return temp_fleet_spec_; 
}
