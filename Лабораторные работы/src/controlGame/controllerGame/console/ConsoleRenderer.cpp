#include "ConsoleRenderer.h"
#include <cstdio>
#include <sstream>
#include <algorithm>


ConsoleRenderer::ConsoleRenderer() = default;

ConsoleRenderer::~ConsoleRenderer() {
    HideCursor(false);
    std::cout << "\x1b[0m\n";
}

void ConsoleRenderer::Initialize() {
    ClearScreenFull();
    HideCursor(true);
}

void ConsoleRenderer::ClearLog(){
    player_log_.clear();
    enemy_log_.clear();
    error_message_.clear();
}

void ConsoleRenderer::Render(const Game& game, const std::string& controls_legend) {
    GameStatus status = game.game_status();

    int current_round = game.round();
    if (last_round_ != current_round) {
        ClearLog();
        last_round_ = current_round;
    }

    if (game.ShouldShowHelp()) {
        RenderHelp(game);
        return; 
    }

    switch (status) {
        case GameStatus::SET_FIELD:
            RenderFieldSizeSelection(game, choice_lines(controls_legend, "ВЫБОР"));
            return;
            
        case GameStatus::SET_SIZES:
            RenderShipSizeSelection(game, choice_lines(controls_legend, "ВЫБОР"));
            return;
            
        case GameStatus::ASK_EXIT:
            RenderQuestionDialog("Вы точно хотите выйти из игры?", choice_lines(controls_legend, "ВЫБОР"));
            return;
            
        case GameStatus::ASK_SAVE:
            RenderQuestionDialog("Сохранить текущую игру?", choice_lines(controls_legend, "ВЫБОР"));
            return;
            
        case GameStatus::SETTING_SHIPS:
            RenderQuestionDialog("Изменить поле или корабли?", choice_lines(controls_legend, "ВЫБОР"));
            return;
            
            
        case GameStatus::SELECT_LOAD_SLOT:
        case GameStatus::SELECT_SAVE_SLOT:
            RenderDialogFiles(game, choice_lines(controls_legend, "ВЫБОР"));
            RenderSaveLoadMessage();   
            return;
            
        default:
            break;
    }

    RenderShipsInfo(game); 
    curr_frame_ = BuildFrame(game, controls_legend);
    
    RenderStats(game);  

    PrintDiff(curr_frame_);
    prev_frame_  = curr_frame_;
    first_frame_ = false;
    std::cout.flush();
}



void ConsoleRenderer::RenderField(const PlayingField& /*board*/, const std::string& /*title*/) {}


void ConsoleRenderer::OnAttackResult(const AttackResult& result) {
    OnAttackResult(result, true);
}

void ConsoleRenderer::OnAttackResult(const AttackResult& result, bool on_enemy_field) {
    std::string side = on_enemy_field ? "[Поле противника]" : "[Ваше поле]";
    const int code = result.hit; // -1 ошибка/повтор, 0 мимо, 1 попадание, 2 потопление
    
    std::string message;
    std::string color_code;
    
    if (code < 0) {
        message = side + " Недействительный выстрел (" + std::to_string(result.x) + "," + std::to_string(result.y) + ")";
        color_code = "\x1b[33m"; 
    } else if (code == 0) {
        message = side + " Промах (" + std::to_string(result.x) + "," + std::to_string(result.y) + ")";
        color_code = "\x1b[91m"; 
    } else if (code == 1) {
        message = side + " Попадание (" + std::to_string(result.x) + "," + std::to_string(result.y) + ")";
        color_code = "\x1b[96m"; 
    } else {
        message = side + " Корабль потоплён (" + std::to_string(result.x) + "," + std::to_string(result.y) + ")";
        color_code = "\x1b[92m";
    }

    AddMessage(color_code + message + "\x1b[0m");
}

void ConsoleRenderer::OnAbilityResult(const AbilityResult& result) {
    std::ostringstream os;
    os << "Способность: " << result.ability_name;
    if (result.x != -1) os << " @(" << result.x << "," << result.y << ")";
    AddMessage(os.str());
}

void ConsoleRenderer::ShowShotBanner(const std::string& text, bool on_enemy_field) {
    std::string side = on_enemy_field ? "[Поле противника]" : "[Ваше поле]";
    AddMessage(side + " " + text);
}


void ConsoleRenderer::ShowMessage(const std::string& message) { 
    AddMessage(message); 
}

void ConsoleRenderer::AddMessage(const std::string& message) {
    if (message.find("Ваше") != std::string::npos) {
        player_log_.push_back(message);
        if (player_log_.size() > max_log_lines_) {
            player_log_.erase(player_log_.begin());
        }
    } else if (message.find("ошибк") != std::string::npos || 
               message.find("Ошибк") != std::string::npos ||
               message.find("недействительн") != std::string::npos) {
        error_message_ = message;
    } else if (message.find("загружен") != std::string::npos || 
               message.find("сохранен") != std::string::npos ||
               message.find("Загружен") != std::string::npos ||
               message.find("Сохранен") != std::string::npos || 
               message.find("слот") != std::string::npos) {
        save_load_message_ = message;
        show_save_load_message_ = 2;
    } else {
        enemy_log_.push_back(message);
        if (enemy_log_.size() > max_log_lines_) {
            enemy_log_.erase(enemy_log_.begin());
        }
    }

    PushLog(message);
}


static void printColoredChar(std::ostringstream& os, char ch) {
    switch (ch) {
        case '.': os << "\x1b[2m"  << ch << "\x1b[0m"; break; 
        case '~': os << "\x1b[34m" << ch << "\x1b[0m"; break;
        case 'O': os << "\x1b[37m" << ch << "\x1b[0m"; break;
        case 'd': os << "\x1b[33m" << ch << "\x1b[0m"; break; 
        case '#': os << "\x1b[31m" << ch << "\x1b[0m"; break; 
        case 'S': os << "\x1b[37m" << ch << "\x1b[0m"; break; 
        default:  os << ch; break;
    }
}

char ConsoleRenderer::CellGlyph(const PlayingField& field, int x, int y, bool reveal_ships) const {
    // поле игрока: знаем всё — различаем INTACT / DAMAGED / DESTROYED
    if (reveal_ships) {
        int ship_index, segment_index, ship_size;
        SegmentState segment_state;
        Orientation  orientation;
        if (field.ship_info_at(x, y, ship_index, segment_index, ship_size, segment_state, orientation)) {
            if (segment_state == SegmentState::DESTROYED) return '#';
            if (segment_state == SegmentState::DAMAGED)   return 'd';
            return 'O';
        }
        const Cell& vis = field.visible_cell(x, y);
        if (vis.IsEmpty())   return '~';
        if (vis.IsUnknown()) return '.';
        return 'S';
    }

    // поле противника:
    const Cell& vis = field.visible_cell(x, y);

    // (1) если уже реально открыто выстрелом
    if (!vis.IsUnknown()) {
        if (vis.IsEmpty()) return '~'; // промах
        // попадание: показываем Damaged/destroyed по реальному состоянию этого сегмента
        int ship_index, segment_index, ship_size;
        SegmentState segment_state;
        Orientation  orientation;
        if (field.ship_info_at(x, y, ship_index, segment_index, ship_size, segment_state, orientation)) {
            if (segment_state == SegmentState::DESTROYED) return '#';
            if (segment_state == SegmentState::DAMAGED)   return 'd';
            return 'S';
        }
        return 'S';
    }

    // (2) если клетка только отсканирована — показать содержимое, но это не выстрел
    if (field.IsScanned(x, y)) {
        return field.IsShipCell(x, y) ? 'S' : '~';
    }

    // (3) неизвестно
    return '.';
}



std::vector<std::string> ConsoleRenderer::BuildFieldBlock(const PlayingField& field, const std::string& title, int cursor_x_, int cursor_y_, bool highlightCursor, bool revealships_) {
    std::vector<std::string> out;
    const int W = field.x_size();
    const int H = field.y_size();
    const int colW = std::max(1, MaxLabelWidthForX(W));
    const int rowW = std::max(2, DigitsCount(H));

    out.push_back(title);

    // шапка
    std::ostringstream os;
    os << std::setw(rowW) << " " << " ";
    for (int x = 0; x < W; ++x) {
        std::string lab = ColumnLabel(x);
        if ((int)lab.size() < colW) {
            lab.insert(lab.begin(), colW - (int)lab.size(), ' ');
        }
        os << " " << lab;
    }
    out.push_back(os.str());

    for (int y = 0; y < H; ++y) {
        std::ostringstream os;
        os << std::setw(rowW) << y << " ";
        for (int x = 0; x < W; ++x) {
            char ch = CellGlyph(field, x, y, revealships_);
            bool isCur = (highlightCursor && x == cursor_x_ && y == cursor_y_);

            os << " ";
            if (isCur) {
                os << "\x1b[7m\x1b[97m";
                printColoredChar(os, ch);
                os << "\x1b[0m";
            } else {
                printColoredChar(os, ch);
            }


            if (colW > 1) os << std::string(colW - 1, ' ');
        }
        out.push_back(os.str());
    }

    return out;
}


std::string ConsoleRenderer::StatusToString(GameStatus status, PlacementMode mode) const {
    switch (status) {
        case GameStatus::PLACING_SHIPS: 
            return (mode == PlacementMode::AUTO) ? "\x1b[1;36mРасстановка кораблей (Авто)\x1b[0m" : "\x1b[1;36mРасстановка кораблей (Ручная)\x1b[0m";
        case GameStatus::PLAYER_TURN:   
            return "\x1b[1;32mВаш ход\x1b[0m"; 
        case GameStatus::ENEMY_TURN:    
            return "\x1b[1;33mХод противника\x1b[0m";
        case GameStatus::PLAYER_WON:    
            return "\x1b[1;34mВы победили!\x1b[0m";
        case GameStatus::ENEMY_WON:     
            return "\x1b[1;31mПротивник победил\x1b[0m";
        case GameStatus::PAUSED:        
            return "\x1b[1;35mПауза\x1b[0m";
        case GameStatus::GAME_OVER:     
            return "\x1b[1;94mИгра завершена\x1b[0m";
        case GameStatus::SET_FIELD:     
            return "\x1b[1;96mНастройка поля\x1b[0m";
        case GameStatus::SET_SIZES:     
            return "\x1b[1;92mНастройка флота\x1b[0m";
        case GameStatus::ASK_SAVE:      
            return "\x1b[1;93mСохранение игры\x1b[0m";
        case GameStatus::SETTING_SHIPS: 
            return "\x1b[1;95mИзменение кораблей\x1b[0m";
        case GameStatus::ASK_EXIT:      
            return "\x1b[1;91mПодтверждение выхода\x1b[0m";
        case GameStatus::WAITING_NEXT_ROUND: 
            return "\x1b[1;37mОжидание начала следующего раунда\x1b[0m";
        case GameStatus::SELECT_LOAD_SLOT:   
            return "\x1b[1;32mВыбор слота загрузки\x1b[0m";
        case GameStatus::SELECT_SAVE_SLOT:   
            return "\x1b[1;33mВыбор слота сохранения\x1b[0m"; 
        default: 
            return "\x1b[1;31mНеизвестный статус\x1b[0m"; 
    }
}




std::vector<std::string> ConsoleRenderer::RenderBaseFields(const Game& game, int player_cursor_x, int player_cursor_y, 
                                                         int enemy_cursor_x, int enemy_cursor_y,
                                                         bool show_player_cursor, bool show_enemy_cursor) {
    std::vector<std::string> lines;

    std::string player_title = "\x1b[1;97m" + game.name() + " (Вы):\x1b[0m";
    std::string enemy_title = "\x1b[1;97m" + game.ai_name() + " (Противник):\x1b[0m";

    auto player_block = BuildFieldBlock(game.player_field(), player_title, 
                                      player_cursor_x, player_cursor_y, show_player_cursor, true);
    lines.insert(lines.end(), player_block.begin(), player_block.end());

    lines.push_back(std::string());
    lines.push_back(std::string());

    auto enemy_block = BuildFieldBlock(game.enemy_field(), enemy_title, 
                                     enemy_cursor_x, enemy_cursor_y, show_enemy_cursor, false);
    lines.insert(lines.end(), enemy_block.begin(), enemy_block.end());

    return lines;
}


std::vector<std::string> ConsoleRenderer::AddRightPanel(const std::vector<std::string>& left_block, const std::vector<std::string>& right_info) {
    std::vector<std::string> result;
    size_t max_lines = std::max(left_block.size(), right_info.size());

    for (size_t i = 0; i < max_lines; ++i) {
        std::ostringstream line;
        
        if (i < left_block.size()) {
            line << left_block[i];
        } else {
            line << std::string(left_block[0].length(), ' ');
        }
        
        line << "    ";
        
        if (i < right_info.size()) {
            line << right_info[i];
        }
        
        result.push_back(line.str());
    }
    
    return result;
}




std::vector<std::string> ConsoleRenderer::BuildFrame(const Game& game, const std::string& controls_legend) {
    std::vector<std::string> lines;
    const int cx = game.current_state().cursor_x();
    const int cy = game.current_state().cursor_y();
    const GameStatus status = game.game_status();

    auto compact_stats = RenderStats(game);

    { 
        std::ostringstream t0, t1, t2;
                 
        t0 << "============================== \x1b[1;97mМОРСКОЙ БОЙ\x1b[0m ===============================";
        t1 << "Раунд: " << game.current_state().round_number()
           << "       Счёт: " << game.player_score() << " - " << game.enemy_score();
        
        
        if (status == GameStatus::PLACING_SHIPS && game.placement_mode() == PlacementMode::MANUAL) {
            t2 << "Курсор на вашем поле: " << ColumnLabel(cx)
               << std::setw(2) << std::setfill('0') << (cy + 1)
               << "  (x=" << cx << ", y=" << cy << ")";
        } else {
            t2 << "Курсор на поле противника: " << ColumnLabel(cx)
               << std::setw(2) << std::setfill('0') << (cy + 1)
               << "  (x=" << cx << ", y=" << cy << ")";
        }
        
        lines.push_back(t0.str());
        lines.push_back(t1.str());
        lines.push_back(t2.str());
        
        lines.push_back("--------------------------------------------------------------------------");
        lines.push_back("неизвестно \x1b[2m.\x1b[0m | вода \x1b[34m~\x1b[0m | корабль \x1b[37mO\x1b[0m | подстрелен \x1b[33md\x1b[0m | потоплен \x1b[31m#\x1b[0m | сегмент \x1b[37mS\x1b[0m");
        lines.push_back("--------------------------------------------------------------------------");   
        
    }

    bool show_cursor_on_player_field = (status == GameStatus::PLACING_SHIPS && game.placement_mode() == PlacementMode::MANUAL);
    bool show_cursor_on_enemy_field = !show_cursor_on_player_field;

    auto base_fields = RenderBaseFields(game,
                                      show_cursor_on_player_field ? cx : -1,
                                      show_cursor_on_player_field ? cy : -1,
                                      show_cursor_on_enemy_field ? cx : -1,
                                      show_cursor_on_enemy_field ? cy : -1,
                                      show_cursor_on_player_field,
                                      show_cursor_on_enemy_field);

    int player_field_height = game.player_field().y_size() + 2;
    std::vector<std::string> player_field_only(base_fields.begin(), 
                                           base_fields.begin() + player_field_height);
    std::vector<std::string> enemy_field_only(base_fields.begin() + player_field_height, 
                                          base_fields.end());

    std::vector<std::string> player_right_panel;
    
    if (status == GameStatus::PLACING_SHIPS) {
        if (game.ShouldShowShipsInfo() && !current_ships_info_.empty()) {
            auto combined_player_field = AddRightPanel(player_field_only, current_ships_info_);
            lines.insert(lines.end(), combined_player_field.begin(), combined_player_field.end());
        } else {
            lines.insert(lines.end(), player_field_only.begin(), player_field_only.end());
        }
    } else {
        player_right_panel.push_back("");
        player_right_panel.push_back("=== ДЕЙСТВИЯ ПРОТИВНИКА ===");
        
        if (player_log_.empty()) {
            player_right_panel.push_back("(пока пусто)");
        } else {
            for (const auto& m : player_log_) {
                player_right_panel.push_back(m);
            }
        }
        
        
        
        auto combined_player_field = AddRightPanel(player_field_only, player_right_panel);
        lines.insert(lines.end(), combined_player_field.begin(), combined_player_field.end());
    }

    if (status == GameStatus::PLACING_SHIPS) {
        if (game.placement_mode() == PlacementMode::MANUAL && game.CanPlaceShip()) {
            auto [ship_size, orientation] = game.current_ship_info();
            std::string orient_str = (orientation == Orientation::HORIZONTAL) ? "горизонтальная" : "вертикальная";
            
            lines.push_back("");
            lines.push_back("Текущий корабль: размер - " + std::to_string(ship_size) + ", ориентация - " + orient_str);
        }
        else{
            lines.push_back(std::string());
            lines.push_back(std::string());
        }
    } else {
        lines.push_back("");
        lines.push_back("\x1b[96mСледующая способность - " + game.ShowAbility() + "\x1b[0m");
    }


    if (status != GameStatus::PLACING_SHIPS) {
        std::vector<std::string> enemy_right_panel;
        enemy_right_panel.push_back("");
        enemy_right_panel.push_back("");
        enemy_right_panel.push_back("");
        enemy_right_panel.push_back("========= ВАШИ ДЕЙСТВИЯ =========");

        if (enemy_log_.empty()) {
            enemy_right_panel.push_back("(пока пусто)");
        } else {
            for (const auto& m : enemy_log_) {
                enemy_right_panel.push_back(m);
            }
        }
        
        auto combined_enemy_field = AddRightPanel(enemy_field_only, enemy_right_panel);
        lines.insert(lines.end(), combined_enemy_field.begin(), combined_enemy_field.end());
    } else {
        lines.insert(lines.end(), enemy_field_only.begin(), enemy_field_only.end());
    }

    lines.push_back(std::string());
    lines.push_back("Статус: " + StatusToString(status, game.placement_mode()));
    lines.push_back("--------------------------------------------------------------------------");
    
    std::vector<std::string> control_lines;
    std::istringstream iss(controls_legend);
    std::string line;
    while (std::getline(iss, line)) {
        control_lines.push_back(line);
    }
    
    lines.insert(lines.end(), control_lines.begin(), control_lines.end());

    if (!compact_stats.empty()) {
        lines.push_back(std::string());
        lines.insert(lines.end(), compact_stats.begin(), compact_stats.end());
    }

    if (!error_message_.empty()) {
        lines.push_back(std::string());
        lines.push_back("\x1b[91m" + error_message_ + "\x1b[0m");
        error_message_.clear();
    }

    if (show_save_load_message_ > 0) {
        lines.push_back(std::string());
        lines.push_back("\x1b[35m" + save_load_message_ + "\x1b[0m");
        show_save_load_message_--;
    }

    if (status == GameStatus::WAITING_NEXT_ROUND) {
        lines.push_back(std::string());
        lines.push_back("\x1b[33mДля следующего раунда нажмите [Y]\x1b[0m");
    }

    return lines;
}




void ConsoleRenderer::RenderDialogFiles(const Game& game, const std::string& controls_legend) {
    auto status = game.game_status();
    ClearScreenFull();

    std::cout << "\x1b[35m" << "======= " 
              << (status == GameStatus::SELECT_SAVE_SLOT ? "СОХРАНИТЬ В СЛОТ" : "ЗАГРУЗИТЬ СЛОТ") 
              << " =======\x1b[0m\n";

    std::cout << "Выберите слот:\n";
    
    for (int i = 0; i < 4; ++i) {
        const std::string slot_name = "slot" + std::to_string(i + 1);
        std::string date = game.current_state().slot_date(slot_name);
        
        std::cout << "- Выб_" <<  std::to_string(i + 1) << ": ";
        
        if (date.empty()) {
            std::cout << "\x1b[90mПусто\x1b[0m";
        } else {
            std::cout << "\x1b[32m" << date << "\x1b[0m";
        }
        
        std::cout << "\n";
    }

    if (status == GameStatus::SELECT_LOAD_SLOT){
        std::string exit_save_date = game.current_state().slot_date("exit_save");
        std::cout << "- Выб_5: ";
        
        if (exit_save_date.empty()) {
            std::cout << "\x1b[90mПусто (сохр. после выхода)\x1b[0m\n";
        } else {
            std::cout << "\x1b[33m" << exit_save_date << " (сохр. после выхода)\x1b[0m\n";
        }
    }
    std::cout << controls_legend;
    std::cout.flush();
}




void ConsoleRenderer::RenderShipsInfo(const Game& game) {
    if (!game.ShouldShowShipsInfo()) {
        current_ships_info_.clear();
        return;
    }
    
    auto ships_info = game.human_player_ships_info();
    current_ships_info_.clear();
    int max_column_height = game.player_field().y_size() + 2;

    std::vector<std::vector<std::string>> columns;
    std::vector<std::string> current_column;
    
    current_column.push_back("");
    current_column.push_back("=== РАЗМЕЩЕНИЕ КОРАБЛЕЙ ===");
    
    for (const auto& ship : ships_info) {    
        if (ship.is_placed) {
            std::string ship_text = "Корабль " + std::to_string(ship.number) + 
                              " (" + std::to_string(ship.size) + "): ";
            std::string orient_str = (ship.orientation == Orientation::HORIZONTAL) ? 
                                  "гор." : "вер.";
            ship_text += "(" + std::to_string(ship.start_pos.x) + "," + 
                       std::to_string(ship.start_pos.y) + "), " + orient_str;
            
           current_column.push_back(ship_text);
        } 
        
        if (current_column.size() >= static_cast<size_t>(max_column_height) && &ship != &ships_info.back()) {
            columns.push_back(current_column);
            current_column.clear();
            current_column.push_back("");
            current_column.push_back("");
        }
    }
    
    if (!current_column.empty()) {
        columns.push_back(current_column);
    }
    
    std::vector<size_t> column_width(columns.size(), 0);
    for (size_t col = 0; col < columns.size(); col++) {
        for (const auto& line : columns[col]) {
            column_width[col] = std::max(column_width[col], line.length());
        }
    }
    
    size_t max_rows = 0;
    for (const auto& col : columns) {
        max_rows = std::max(max_rows, col.size());
    }
    
    for (size_t row = 0; row < max_rows; row++) {
        std::string combined_row;
        for (size_t col = 0; col < columns.size(); col++) {
            if (row < columns[col].size()) {
                std::string line = columns[col][row];
                line += std::string(column_width[col] - line.length(), ' ');
                combined_row += line;
            } else {
                combined_row += std::string(column_width[col], ' ');
            }
            
            if (col < columns.size() - 1) {
                combined_row += "    ";
            }
        }
        current_ships_info_.push_back(combined_row);
    }
}



void ConsoleRenderer::PrintDiff(const std::vector<std::string>& next) {
    ClearScreenFull();
    MoveCursor(1, 1);
    for (const auto& s : next) std::cout << s << '\n';
    std::cout.flush();
}

void ConsoleRenderer::ClearScreenFull() { std::cout << "\x1b[2J\x1b[H"; }
void ConsoleRenderer::HideCursor(bool hide) { std::cout << (hide ? "\x1b[?25l" : "\x1b[?25h"); }

void ConsoleRenderer::MoveCursor(int row, int col) {
    row = std::max(1, row);
    col = std::max(1, col);
    std::cout << "\x1b[" << row << ";" << col << "H";
}

void ConsoleRenderer::PushLog(const std::string& msg) {
    if (msg.empty()) return;
    log_.push_back(msg);
    while (log_.size() > log_max_) log_.pop_front();
}


void ConsoleRenderer::RenderSaveLoadMessage() {
    if (show_save_load_message_ > 0) {
        std::cout << "\n\x1b[35m" << save_load_message_ << "\x1b[0m\n";
    }
}


std::vector<std::string> ConsoleRenderer::RenderStats(const Game& game) {
    std::vector<std::string> stats_lines;
    
    if (!game.ShouldShowStats()) {
        return stats_lines;
    }
    
    const std::string stats = game.statistics();
    std::istringstream iss(stats);
    std::string line;
     
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            std::string colored_line = line;
            
            if (line.find("СТАТИСТИКА РАУНДА") != std::string::npos) {
                colored_line = "\x1b[1;97m" + line + "\x1b[0m";
            }
            else if (  line.find("ОБЩАЯ СТАТИСТИКА") != std::string::npos){
                colored_line = "\n\x1b[1;97m" + line + "\x1b[0m";
            }
            else if (line.find("Победитель:") != std::string::npos) {
                colored_line = "\x1b[1;33m" + line + "\x1b[0m";
            }
            else if (line.find("Счёт:") != std::string::npos) {
                colored_line = "\x1b[92m" + line + "\x1b[0m";
            }
            else if (line.find("Вы  |") != std::string::npos || line.find("Вы |") != std::string::npos) {
                colored_line = "\x1b[96m" + line + "\x1b[0m";
            }
            else {
                colored_line = "\x1b[95m" + line + "\x1b[0m";
            }
            
            stats_lines.push_back(colored_line);
        }
    }

    
    return stats_lines;
}




void ConsoleRenderer::RenderHelp(const Game& game) {
    ClearScreenFull();
    std::cout << "\x1b[1;97m" << " ========================= ПОМОЩЬ - МОРСКОЙ БОЙ ========================= " << "\x1b[0m\n";
    std::string help_text = game.game_help();
    std::stringstream ss(help_text);
    std::string line;
    
    while (std::getline(ss, line)) {
        std::cout << line << "\n";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


void ConsoleRenderer::RenderQuestionDialog(const std::string& question, const std::string& controls_legend) {
    ClearScreenFull();
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "\x1b[1;97m " << question << "\x1b[0m\n";
    std::cout << "--------------------------------------------" << std::endl;
    
    auto buttons = ExtractAllButtons(controls_legend);
    std::string buttons_text = "[Y]/[N] - Да/Нет";

    for (const auto& [key, value] : buttons) {
        if (key.find("]/[") != std::string::npos) {
            buttons_text = key + " - " + value;
            break;
        }
    }
    
    std::cout << "\x1b[1;97m " << buttons_text << "\x1b[0m\n\n";
    std::cout.flush();
}



void ConsoleRenderer::RenderFieldSizeSelection(const Game& game, const std::string& controls_legend) {
    ClearScreenFull();
    auto base_fields = RenderBaseFields(game, -1, -1, -1, -1, false, false);
    
    std::vector<std::string> size_info = {
        "",
        "===== ВЫБОР РАЗМЕРА ПОЛЯ =====",
        "Выберите размер игрового поля:",
        "- Выб_1: 10×10 клеток",
        "- Выб_2: 11×11 клеток", 
        "- Выб_3: 12×12 клеток",
        "- Выб_4: 13×13 клеток",
        "- Выб_5: 14×14 клеток",
        "", 
        "Выбранный размер: " + std::to_string(game.temp_field_size()),
        "",
        controls_legend
    };

    std::vector<std::string> player_field_only(base_fields.begin(), base_fields.begin() + game.player_field().y_size() + 2);
    auto combined = AddRightPanel(player_field_only, size_info);

    for (const auto& line : combined) {
        std::cout << line << "\n";
    }
    for (size_t i = player_field_only.size(); i < base_fields.size(); ++i) {
        std::cout << base_fields[i] << "\n";
    }
    
    std::cout.flush();
}



void ConsoleRenderer::RenderShipSizeSelection(const Game& game, const std::string& controls_legend) {
    ClearScreenFull();
    auto base_fields = BuildFieldBlock(game.player_field(), "Ваше поле:", -1, -1, false, true);
    
    std::vector<std::string> fleet_info = {
        "",
        "======= СОЗДАНИЕ ФЛОТА =======",
        "Добавьте корабли в свой флот (макс 16):",
        "- Выб_1: корабль 1×1",
        "- Выб_2: корабль 1×2", 
        "- Выб_3: корабль 1×3",
        "- Выб_4: корабль 1×4",
        "- Выб_5: стандартный флот (1x4,2x3,3x2,4x1)",
        "",
        "Созданный флот: " + game.fleet_spec_string(true),
        "Текущий флот: " + game.fleet_spec_string(),
        controls_legend
    };

    std::vector<std::string> player_field_only(base_fields.begin(), base_fields.begin() + game.player_field().y_size() + 2);
    auto combined = AddRightPanel(player_field_only, fleet_info);
    
    for (const auto& line : combined) {
        std::cout << line << "\n";
    }
    for (size_t i = player_field_only.size(); i < base_fields.size(); ++i) {
        std::cout << base_fields[i] << "\n";
    }
    
    std::cout.flush();
}


std::vector<std::pair<std::string, std::string>> ConsoleRenderer::ExtractAllButtons(const std::string& str) {
        std::vector<std::pair<std::string, std::string>> buttons;
        
        std::regex pattern(R"(\[([^\]]+)\](?:\s*\/\s*\[([^\]]+)\])?\s+([^\/\|\]]+(?:\s+[^\/\|\]]+)*)(?:\s*\/\s*([^\/\|\]]+(?:\s+[^\/\|\]]+)*))?)");
        
        auto begin = std::sregex_iterator(str.begin(), str.end(), pattern);
        auto end = std::sregex_iterator();
        
        for (std::sregex_iterator i = begin; i != end; ++i) {
            std::smatch match = *i;
            
            auto Trim = [](std::string s) {
                s.erase(0, s.find_first_not_of(" \t"));
                s.erase(s.find_last_not_of(" \t") + 1);
                return s;
            };
            
            if (match[2].matched) {
                std::string keys = Trim(match[1].str()) + "/" + Trim(match[2].str());
                std::string values = Trim(match[3].str()) + "/" + Trim(match[4].str());
                buttons.emplace_back(keys, values);
            } else {
                std::string key = Trim(match[1].str());
                std::string value = Trim(match[3].str());
                buttons.emplace_back(key, value);
            }
        }
        
        return buttons;
    }


    std::string ConsoleRenderer::choice_lines(const std::string& text, const std::string& substring) {
        std::istringstream iss(text);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.find(substring) != std::string::npos) {
                return line;
            }
        }
        return "";
    }