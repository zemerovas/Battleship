#include "GUIRenderer.h"
#include <iomanip>
#include <algorithm>
#include <functional>

GUIRenderer::GUIRenderer(sf::RenderWindow& window, int cell)
    : window_(window), sound_manager_(SoundManager::getInstance()),
      ai_name_("AI"), cell_size_(cell), cell_spacing_(cell + 5) {}

GUIRenderer::~GUIRenderer() = default;

void GUIRenderer::Initialize() {
    if (!LoadResources()) {
        std::cerr << "Ошибка загрузки ресурсов для GUI!\n";
    }
}

void GUIRenderer::ClearLog(){
    player_log_.clear();
    ai_log_.clear();
    
}

void GUIRenderer::Render(const Game& game, const std::string& controls_legend){
    window_.clear(sf::Color(30, 30, 60));
    const PlayingField player_field = game.player_field();
    const PlayingField enemy_field = game.enemy_field();
    
    int current_round = game.round();
    if (last_round_ != current_round) {
        ClearLog();
        last_round_ = current_round;
    }
    
    grid_width_ = player_field.x_size() * cell_spacing_ + 10.f;
    grid_height_ = player_field.y_size() * cell_spacing_ + 40.f;
    total_width_ = 30.f + grid_width_ + 80.f + grid_width_ + 30.f;
    total_height_ = 100.f + grid_height_ + 150.f;
    
    player_pos_ = sf::Vector2f(offset_x_, offset_y_);
    enemy_pos_ = sf::Vector2f(offset_x_ + player_field.x_size() * cell_spacing_ + 18.f, offset_y_);
    
    log_position_ = sf::Vector2f(enemy_pos_.x + enemy_field.x_size() * cell_spacing_ + 30.f, enemy_pos_.y + 30.f);
    ai_column_x_ = log_position_.x - 5.f;
    player_column_x_ = ai_column_x_ + column_width_ + column_spacing_;
    frame_y_ = log_position_.y - 35.f;
   
    const float sx = static_cast<float>(window_.getSize().x) / total_width_;
    const float sy = static_cast<float>(window_.getSize().y) / total_height_;
    const float fit = std::min(1.f, std::min(sx, sy));
    if (fit < 1.f) {
        sf::View v = window_.getDefaultView();
        v.zoom(1.f / fit);
        window_.setView(v);
    } else {
        window_.setView(window_.getDefaultView());
    }
    
    ai_name_ = game.ai_name();
    RenderGameTitle(game.current_state().round_number());
    RenderScore(game);
    RenderField(player_field, game.name());
    RenderField(enemy_field, ai_name_);
    RenderCursor(game);
    RenderGameStatus(game);
    RenderShipsInfo(game);
    RenderInput(game);
    RenderAbilitiesInfo(game.ShowAbility());
    RenderBanners();
    RenderLog();
    RenderSaveLoadMessage();
    RenderControlsLegend(controls_legend);
    RenderStats(game);
    RenderFieldSizeSelection(game);
    RenderShipSizeSelection(game);
    RenderAskRound(game);
    RenderSettingships_(game);
    RenderAskSave(game);
    RenderDialogFiles(game);
    RenderAskExit(game);
    RenderHelp(game);
    RenderError();
    window_.display();
}

void GUIRenderer::RenderLog() {
    sf::RectangleShape ai_frame(sf::Vector2f(column_width_, column_height_));
    ai_frame.setPosition(ai_column_x_, frame_y_);
    ai_frame.setFillColor(sf::Color(0, 0, 0, 100));
    ai_frame.setOutlineThickness(2);
    ai_frame.setOutlineColor(sf::Color::White);
    window_.draw(ai_frame);

    sf::Text ai_title;
    ai_title.setFont(font_);
    ai_title.setCharacterSize(20);
    ai_title.setStyle(sf::Text::Bold);
    ai_title.setFillColor(sf::Color::Cyan);
    ai_title.setString(utf8(u8"Действия AI"));
    ai_title.setPosition(ai_column_x_ + 5.f, frame_y_ + 5.f);
    window_.draw(ai_title);

    float offset_y = 30.f;
    for (const auto& msg : ai_log_) {
        std::vector<std::string> lines = wrapText(msg, column_width_ - 10.f, 18);
       
        for (const auto& line : lines) {
            sf::Text log_text;
            log_text.setFont(font_);
            log_text.setCharacterSize(18);
           
            if (line.find("Ошибка") != std::string::npos){
                log_text.setFillColor(sf::Color::Red);
            }
            else if (line.find("промах") != std::string::npos) {
                log_text.setFillColor(sf::Color(255, 80, 0));
            }
            else if (line.find("уничтожение") != std::string::npos){
                log_text.setFillColor(sf::Color::Green);
            }
            else if (line.find("повреждение") != std::string::npos){
                log_text.setFillColor(sf::Color::Yellow);
            }
            else {
                log_text.setFillColor(sf::Color::White);
            }
           
            log_text.setString(utf8(line));
            log_text.setPosition(ai_column_x_ + 5.f, frame_y_ + offset_y);
            window_.draw(log_text);
            offset_y += 22.f;
        }
    }
    sf::RectangleShape player_frame(sf::Vector2f(column_width_, column_height_));
    player_frame.setPosition(player_column_x_, frame_y_);
    player_frame.setFillColor(sf::Color(0, 0, 0, 100));
    player_frame.setOutlineThickness(2);
    player_frame.setOutlineColor(sf::Color::White);
    window_.draw(player_frame);

    sf::Text player_title;
    player_title.setFont(font_);
    player_title.setCharacterSize(20);
    player_title.setStyle(sf::Text::Bold);
    player_title.setFillColor(sf::Color::Cyan);
    player_title.setString(utf8(u8"Ваши действия"));
    player_title.setPosition(player_column_x_ + 5.f, frame_y_ + 5.f);
    window_.draw(player_title);

    offset_y = 30.f;
    for (const auto& msg : player_log_) {
        std::vector<std::string> lines = wrapText(msg, column_width_ - 10.f, 18);
       
        for (const auto& line : lines) {
            sf::Text log_text;
            log_text.setFont(font_);
            log_text.setCharacterSize(18);
           
            if (line.find("Ошибка") != std::string::npos){
                log_text.setFillColor(sf::Color::Red);
            }
            else if (line.find("промах") != std::string::npos) {
                log_text.setFillColor(sf::Color(255, 80, 0));
            }
            else if (line.find("уничтожение") != std::string::npos){
                log_text.setFillColor(sf::Color::Green);
            }
            else if (line.find("повреждение") != std::string::npos){
                log_text.setFillColor(sf::Color::Yellow);
            }
            else if (line.find("Скан") != std::string::npos ||
                    line.find("Урон") != std::string::npos ||
                    line.find("Обстрел") != std::string::npos){
                    log_text.setFillColor(sf::Color(120, 80, 230));
            }
            else {
                log_text.setFillColor(sf::Color::White);
            }
           
            log_text.setString(utf8(line));
            log_text.setPosition(player_column_x_ + 5.f, frame_y_ + offset_y);
            window_.draw(log_text);
            offset_y += 22.f;
        }
    }
}

void GUIRenderer::RenderSaveLoadMessage(){
    if (show_save_load_message_ <= 0) return;

    sf::Text save_load_text;
    save_load_text.setFont(font_);
    save_load_text.setCharacterSize(22);
    save_load_text.setFillColor(sf::Color(255, 20, 147));
    save_load_text.setString(utf8(save_load_message_));
    save_load_text.setPosition(player_pos_.x, player_pos_.y + grid_height_ + 43.f);
    window_.draw(save_load_text);

    show_save_load_message_--;

}

void GUIRenderer::RenderInput(const Game& game){
    if (game.game_status() != GameStatus::PLACING_SHIPS || game.placement_mode() != PlacementMode::MANUAL) {
        return;
    }

    sf::RectangleShape input_frame(sf::Vector2f(column_width_, input_height_));
    input_frame.setPosition(ai_column_x_, frame_y_ + column_height_ + ability_height_ + 20.f);
    input_frame.setFillColor(sf::Color(0, 0, 0, 100));
    input_frame.setOutlineThickness(2);
    input_frame.setOutlineColor(sf::Color::White);
    window_.draw(input_frame);

    if (game.CanPlaceShip()) {
        auto [ship_size, orientation] = game.current_ship_info();
       
        std::string orientation_str = (orientation == Orientation::HORIZONTAL) ?
                                   u8"горизонтальная" : u8"вертикальная";
       
        std::string info_text = u8"Размер корабля: " + std::to_string(ship_size) +
                             u8" сегмент.\nОриентация: " + orientation_str;
       
        sf::Text title_text;
        title_text.setFont(font_);
        title_text.setCharacterSize(20);
        title_text.setStyle(sf::Text::Bold);
        title_text.setFillColor(sf::Color::Cyan);
        title_text.setString(utf8("Текущий корабль:"));
        title_text.setPosition(ai_column_x_ + 5.f, frame_y_ + column_height_ + ability_height_ + 25.f);
        window_.draw(title_text);

        sf::Text ship_text;
        ship_text.setFont(font_);
        ship_text.setCharacterSize(18);
        ship_text.setFillColor(sf::Color::White);
        ship_text.setString(utf8(info_text));
        ship_text.setPosition(ai_column_x_ + 5.f, frame_y_ + column_height_ + ability_height_ + 55.f);
        window_.draw(ship_text);
    }
}

void GUIRenderer::RenderAbilitiesInfo(std::string info) {
    sf::RectangleShape ability_frame(sf::Vector2f(column_width_ * 2 + column_spacing_, ability_height_));
    ability_frame.setPosition(ai_column_x_, frame_y_ + column_height_ + 10.f);
    ability_frame.setFillColor(sf::Color(0, 0, 0, 100));
    ability_frame.setOutlineThickness(2);
    ability_frame.setOutlineColor(sf::Color::White);
    window_.draw(ability_frame);

    sf::Text ability_title;
    ability_title.setFont(font_);
    ability_title.setCharacterSize(20);
    ability_title.setStyle(sf::Text::Bold);
    ability_title.setFillColor(sf::Color::Cyan);
    ability_title.setString(utf8("Следующая способность"));
    ability_title.setPosition(ai_column_x_ + 5.f, frame_y_ + column_height_ + 15.f);
    window_.draw(ability_title);

    sf::Text ability_text;
    ability_text.setFont(font_);
    ability_text.setCharacterSize(18);
    if (info.find("Нет") != std::string::npos) {
        ability_text.setFillColor(sf::Color::Red);
    } else {
        ability_text.setFillColor(sf::Color::Green);
    }
    ability_text.setString(utf8(info));
    ability_text.setPosition(ai_column_x_ + 15.f, frame_y_ + column_height_ + 45.f);
    window_.draw(ability_text);
}

void GUIRenderer::RenderShipsInfo(const Game& game) {
    if (!game.ShouldShowShipsInfo()) return;
    auto ships_info = game.human_player_ships_info();
    panel_height_ = 300.f + ((game.ship_count() % 10) * 18.f);
    
    sf::RectangleShape background(sf::Vector2f(column_width_, panel_height_));
    background.setPosition(player_column_x_, frame_y_ + column_height_ + ability_height_ + 20.f);
    background.setFillColor(sf::Color(0, 0, 0, 100));
    background.setOutlineThickness(3);
    background.setOutlineColor(sf::Color::White);
    window_.draw(background);
    
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(20);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color::Cyan);
    title.setString(utf8(u8" Размещение кораблей"));
    title.setPosition(player_column_x_, frame_y_ + column_height_ + ability_height_ + 20.f + 5.f);
    window_.draw(title);
    
    float offset_y = 32.f;
    for (const auto& ship : ships_info) {
        if (ship.is_placed) {
            std::string ship_text = "Корабль " + std::to_string(ship.number) +
                              " (" + std::to_string(ship.size) + "): ";
            std::string orient_str = (ship.orientation == Orientation::HORIZONTAL) ?
                                  "горизонт." : "вертикал.";
            ship_text += "(" + std::to_string(ship.start_pos.x) + "," +
                       std::to_string(ship.start_pos.y) + "), " + orient_str;
           
            sf::Text text;
            text.setFont(font_);
            text.setCharacterSize(18);
            text.setFillColor(sf::Color::White);
            text.setString(utf8(ship_text));
            text.setPosition(player_column_x_ + 6.f, frame_y_ + column_height_ + ability_height_ + 20.f + offset_y);
            window_.draw(text);
            offset_y += 23.f;
        }
    }
}

void GUIRenderer::RenderHelp(const Game& game) {
    if (!game.ShouldShowHelp()) return;
    
    help_x = (window_.getSize().x - help_width) / 2.f;
    help_y = (window_.getSize().y - help_height) / 2.f;
    
    sf::VertexArray gradient(sf::Quads, 4);
    sf::Color bottom_color(5, 10, 30);
    sf::Color top_color(35, 30, 85);
    gradient[0].position = sf::Vector2f(help_x, help_y + help_height);
    gradient[1].position = sf::Vector2f(help_x + help_width, help_y + help_height);
    gradient[2].position = sf::Vector2f(help_x + help_width, help_y);
    gradient[3].position = sf::Vector2f(help_x, help_y);
    gradient[0].color = bottom_color;
    gradient[1].color = bottom_color;
    gradient[2].color = top_color;
    gradient[3].color = top_color;
    window_.draw(gradient);
    
    sf::RectangleShape frame(sf::Vector2f(help_width, help_height));
    frame.setPosition(help_x, help_y);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(100, 120, 255, 160));
    window_.draw(frame);
    
    sf::RectangleShape inner(sf::Vector2f(help_width - 20, help_height - 20));
    inner.setPosition(help_x + 10, help_y + 10);
    inner.setFillColor(sf::Color::Transparent);
    inner.setOutlineThickness(1.5f);
    inner.setOutlineColor(sf::Color(130, 140, 255, 90));
    window_.draw(inner);
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            sf::ConvexShape corner;
            corner.setPointCount(3);
            if (i == 0 && j == 0) {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(30, 0));
                corner.setPoint(2, sf::Vector2f(0, 30));
            } else if (i == 1 && j == 0) {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(-30, 0));
                corner.setPoint(2, sf::Vector2f(0, 30));
            } else if (i == 0 && j == 1) {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(30, 0));
                corner.setPoint(2, sf::Vector2f(0, -30));
            } else {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(-30, 0));
                corner.setPoint(2, sf::Vector2f(0, -30));
            }
            corner.setPosition(help_x + i * help_width, help_y + j * help_height);
            corner.setFillColor(sf::Color(110, 100, 255, 100));
            window_.draw(corner);
        }
    }
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(26);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(240, 230, 255));
    title.setString(utf8(u8"ПОМОЩЬ"));
    title.setPosition(help_x + help_width / 2.f - 50.f, help_y + 15.f);
    window_.draw(title);
    
    std::string help_text = game.game_help();
    std::vector<std::string> lines;
    std::stringstream ss(help_text);
    std::string line;
   
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    float current_y = help_y + 50.f;
    for (const auto& current_line : lines) {
        sf::Text line_text;
        line_text.setFont(font_);
        line_text.setCharacterSize(17);
       
        if (current_line.find("ЦЕЛЬ") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(255, 50, 100));
        } else if (current_line.find("ПРАВИЛА") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(255, 150, 0));
        } else if (current_line.find("ПРОЦЕСС") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(255, 220, 100));
        } else if (current_line.find("СПОСОБНОСТИ") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(50, 255, 150));
        } else if (current_line.find("СОХРАНЕНИЙ") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(0, 200, 255));
        } else if (current_line.find("УПРАВЛЕНИЕ") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(180, 80, 255));
        } else {
            line_text.setFillColor(sf::Color(220, 240, 255));
        }
        line_text.setString(utf8(current_line));
        line_text.setPosition(help_x + 25.f, current_y);
        window_.draw(line_text);
        current_y += 22.f;
    }
}

void GUIRenderer::RenderDialogFiles(const Game& game) {
    auto status = game.game_status();
    if (status != GameStatus::SELECT_LOAD_SLOT && status != GameStatus::SELECT_SAVE_SLOT) return;
    
    dialog_files_x_ = (window_.getSize().x - dialog_files_width_) / 2.f;
    dialog_files_y_ = (window_.getSize().y - dialog_files_height_) / 2.f;
    
    sf::VertexArray gradient(sf::Quads, 4);
    sf::Color bottom_color(90, 25, 40);
    sf::Color top_color(160, 80, 110);
    gradient[0].position = sf::Vector2f(dialog_files_x_, dialog_files_y_ + dialog_files_height_);
    gradient[1].position = sf::Vector2f(dialog_files_x_ + dialog_files_width_, dialog_files_y_ + dialog_files_height_);
    gradient[2].position = sf::Vector2f(dialog_files_x_ + dialog_files_width_, dialog_files_y_);
    gradient[3].position = sf::Vector2f(dialog_files_x_, dialog_files_y_);
    
    gradient[0].color = bottom_color; 
    gradient[1].color = bottom_color;
    gradient[2].color = top_color; 
    gradient[3].color = top_color;
    window_.draw(gradient);
    
    sf::RectangleShape frame(sf::Vector2f(dialog_files_width_, dialog_files_height_));
    frame.setPosition(dialog_files_x_, dialog_files_y_);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(220, 120, 140, 180));
    window_.draw(frame);

    sf::RectangleShape inner_glow(sf::Vector2f(dialog_files_width_ - 20, dialog_files_height_ - 20));
    inner_glow.setPosition(dialog_files_x_ + 10, dialog_files_y_ + 10);
    inner_glow.setFillColor(sf::Color::Transparent);
    inner_glow.setOutlineThickness(1.5f);
    inner_glow.setOutlineColor(sf::Color(255, 160, 180, 100));
    window_.draw(inner_glow);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            sf::ConvexShape corner;
            corner.setPointCount(3);
            if (i == 0 && j == 0) {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(30, 0));
                corner.setPoint(2, sf::Vector2f(0, 30));
            } else if (i == 1 && j == 0) {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(-30, 0));
                corner.setPoint(2, sf::Vector2f(0, 30));
            } else if (i == 0 && j == 1) {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(30, 0));
                corner.setPoint(2, sf::Vector2f(0, -30));
            } else {
                corner.setPoint(0, sf::Vector2f(0, 0));
                corner.setPoint(1, sf::Vector2f(-30, 0));
                corner.setPoint(2, sf::Vector2f(0, -30));
            }
            corner.setPosition(dialog_files_x_ + i * dialog_files_width_, dialog_files_y_ + j * dialog_files_height_);
            corner.setFillColor(sf::Color(180, 70, 100, 130));
            window_.draw(corner);
        }
    }
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(28);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 230, 235));
    title.setString(status == GameStatus::SELECT_SAVE_SLOT ? utf8(u8"СОХРАНИТЬ В СЛОТ") : utf8(u8"ЗАГРУЗИТЬ СЛОТ"));
    title.setPosition(dialog_files_x_ + dialog_files_width_ / 2.f - 128.f, dialog_files_y_ + 20.f);
    window_.draw(title);


    int n = (status == GameStatus::SELECT_LOAD_SLOT) ? 5 : 4;
    slot_start_x = dialog_files_x_ + 25.f + ((n == 4) ? 65.f : 0);
    slot_start_y = dialog_files_y_ + 80.f;
   
    for (int i = 0; i < n; ++i) {
        float slot_x = slot_start_x + i * (slot_width + 20.f);
        const std::string slot_name = (i == 4) ? "exit_save" : "slot" + std::to_string(i + 1);
        std::string date = game.current_state().slot_date(slot_name);

        sf::RectangleShape slot_background(sf::Vector2f(slot_width, slot_height));
        slot_background.setPosition(slot_x, slot_start_y);
        slot_background.setFillColor(sf::Color(120, 40, 60, 190));
        slot_background.setOutlineThickness(1.5f);
        slot_background.setOutlineColor(sf::Color(200, 100, 130));
        window_.draw(slot_background);

        sf::Text slot_num;
        slot_num.setFont(font_);
        slot_num.setCharacterSize(24);
        slot_num.setStyle(sf::Text::Bold);
        slot_num.setFillColor(sf::Color(255, 200, 210));
        slot_num.setString(std::to_string(i + 1));
        slot_num.setOrigin(slot_num.getLocalBounds().width / 2.f, slot_num.getLocalBounds().height / 2.f);
        slot_num.setPosition(slot_x + slot_width / 2.f, slot_start_y + 15.f);
        window_.draw(slot_num);

        sf::Text slot_info;
        slot_info.setFont(font_);
        slot_info.setCharacterSize(14);
        std::string slot_text;

        if (date.empty()) {
            slot_text = u8"Пусто";
            slot_info.setFillColor(sf::Color(220, 255, 220));
        } else {
            slot_text = date;
            slot_info.setFillColor(sf::Color(100, 255, 150));
        }

        slot_info.setString(utf8(slot_text));
        slot_info.setOrigin(slot_info.getLocalBounds().width / 2.f, slot_info.getLocalBounds().height / 2.f);
        slot_info.setPosition(slot_x + slot_width / 2.f, slot_start_y + 45.f);
        window_.draw(slot_info);

        sf::Text slot_label;
        slot_label.setFont(font_);
        slot_label.setCharacterSize(16);
        slot_label.setString(utf8(u8"выб_" + std::to_string(i+1)));
        slot_label.setFillColor(sf::Color(200, 200, 200));
        slot_label.setOrigin(slot_label.getLocalBounds().width / 2.f, slot_label.getLocalBounds().height / 2.f);
        slot_label.setPosition(slot_x + slot_width / 2.f, slot_start_y + slot_height + 20.f);
        window_.draw(slot_label);
    }

    sf::Text hint;
    hint.setFont(font_);
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(180, 200, 230));
    hint.setString(utf8(u8"НЕТ - отмена"));
    hint.setPosition(dialog_files_x_ + 35.f, dialog_files_y_ + dialog_files_height_ - 42.f);
    window_.draw(hint);

    if (n == 5){
        sf::Text save_slot_5;
        save_slot_5.setFont(font_);
        save_slot_5.setCharacterSize(14);
        save_slot_5.setFillColor(sf::Color(180, 200, 230));
        save_slot_5.setString(utf8(u8"5 слот: сохранение после выхода"));
        save_slot_5.setPosition(dialog_files_x_ + 3.2f * dialog_files_width_ / 5, dialog_files_y_ + dialog_files_height_ - 42.f);
        window_.draw(save_slot_5);
    }
}

void GUIRenderer::RenderAskWindow(const std::string& info){
    ask_window_x = (window_.getSize().x - dialog_width_) / 2.f;
    ask_window_y = (window_.getSize().y - dialog_height_) / 2.f;
    sf::VertexArray gradient(sf::Quads, 4);
    sf::Color top_color(15, 35, 75);
    sf::Color bottom_color(65, 125, 195);

    gradient[0].position = sf::Vector2f(ask_window_x, ask_window_y);
    gradient[1].position = sf::Vector2f(ask_window_x + dialog_width_, ask_window_y);
    gradient[2].position = sf::Vector2f(ask_window_x + dialog_width_, ask_window_y + dialog_height_);
    gradient[3].position = sf::Vector2f(ask_window_x, ask_window_y + dialog_height_);

    gradient[0].color = top_color;
    gradient[1].color = top_color;
    gradient[2].color = bottom_color;
    gradient[3].color = bottom_color;
    window_.draw(gradient);
   
    sf::Color inner_frame_color(100, 180, 255, 200);
    sf::RectangleShape top_line(sf::Vector2f(dialog_width_ - 2 * ask_window_inner_offset, ask_window_inner_thickness));
    top_line.setPosition(ask_window_x + ask_window_inner_offset, ask_window_y + ask_window_inner_offset);
    top_line.setFillColor(inner_frame_color);
    window_.draw(top_line);
   
    sf::RectangleShape bottom_line(sf::Vector2f(dialog_width_ - 2 * ask_window_inner_offset, ask_window_inner_thickness));
    bottom_line.setPosition(ask_window_x + ask_window_inner_offset, ask_window_y + dialog_height_ - ask_window_inner_offset - ask_window_inner_thickness);
    bottom_line.setFillColor(inner_frame_color);
    window_.draw(bottom_line);
   
    sf::RectangleShape left_line(sf::Vector2f(ask_window_inner_thickness, dialog_height_ - 2 * ask_window_inner_offset));
    left_line.setPosition(ask_window_x + ask_window_inner_offset, ask_window_y + ask_window_inner_offset);
    left_line.setFillColor(inner_frame_color);
    window_.draw(left_line);
   
    sf::RectangleShape right_line(sf::Vector2f(ask_window_inner_thickness, dialog_height_ - 2 * ask_window_inner_offset));
    right_line.setPosition(ask_window_x + dialog_width_ -ask_window_inner_offset - ask_window_inner_thickness, ask_window_y + ask_window_inner_offset);
    right_line.setFillColor(inner_frame_color);
    window_.draw(right_line);

    sf::RectangleShape frame(sf::Vector2f(dialog_width_, dialog_height_));
    frame.setPosition(ask_window_x, ask_window_y);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(70, 140, 220));
    window_.draw(frame);

    sf::Text hint_text;
    hint_text.setFont(font_);
    hint_text.setString(utf8(u8"ДА/НЕТ из управления"));
    hint_text.setCharacterSize(16);
    hint_text.setFillColor(sf::Color(220, 245, 255));

    sf::FloatRect hint_bounds = hint_text.getLocalBounds();
    hint_text.setPosition(
        ask_window_x + (dialog_width_ - hint_bounds.width) / 2.f,
        ask_window_y + dialog_height_ - 35.f
    );
    window_.draw(hint_text);

    sf::Text message_text;
    message_text.setFont(font_);
    message_text.setString(utf8(info));
    message_text.setCharacterSize(24);
    message_text.setFillColor(sf::Color(200, 240, 220));
    message_text.setStyle(sf::Text::Bold);
   
    sf::FloatRect text_bounds = message_text.getLocalBounds();
    message_text.setPosition(
        ask_window_x + (dialog_width_ - text_bounds.width) / 2.f,
        ask_window_y + (dialog_height_ - text_bounds.height) / 2.f - 10.f
    );
    window_.draw(message_text);
}


void GUIRenderer::RenderAskRound(const Game& game){
    if (game.game_status() != GameStatus::WAITING_NEXT_ROUND) return;
    sf::Text hint_text;
    hint_text.setFont(font_);
    hint_text.setCharacterSize(22);
    hint_text.setFillColor(sf::Color(230, 210, 255));
    hint_text.setString(utf8("Для следующего раунда нажми [Y]"));
    hint_text.setPosition(player_pos_.x, player_pos_.y + grid_height_ + 40.f);
    window_.draw(hint_text);
}

void GUIRenderer::RenderSettingships_(const Game& game){
    if (game.game_status() != GameStatus::SETTING_SHIPS) return;
   RenderAskWindow(u8"Изменить поле или корабли?");
}

void GUIRenderer::RenderAskSave(const Game& game){
    if (game.game_status() != GameStatus::ASK_SAVE) return;
    RenderAskWindow(u8"Сохранить игру?");
}

void GUIRenderer::RenderAskExit(const Game& game){
    if (game.game_status() != GameStatus::ASK_EXIT) return;
    RenderAskWindow(u8"Вы точно хотите выйти?");
}

void GUIRenderer::RenderFieldSizeSelection(const Game& game) {
    if (game.game_status() != GameStatus::SET_FIELD) return;
    field_select_x_ = (window_.getSize().x - field_select_width_) / 2.f;
    field_select_y_ = (window_.getSize().y - field_select_height_) / 2.f;
    const float padding = 30.f;

    sf::VertexArray gradient(sf::Quads, 4);
    sf::Color topColor(40, 20, 60);
    sf::Color bottomColor(80, 40, 100);

    gradient[0].position = sf::Vector2f(field_select_x_, field_select_y_);
    gradient[1].position = sf::Vector2f(field_select_x_ + field_select_width_, field_select_y_);
    gradient[2].position = sf::Vector2f(field_select_x_ + field_select_width_, field_select_y_ + field_select_height_);
    gradient[3].position = sf::Vector2f(field_select_x_, field_select_y_ + field_select_height_);

    gradient[0].color = topColor;
    gradient[1].color = topColor;
    gradient[2].color = bottomColor;
    gradient[3].color = bottomColor;
    window_.draw(gradient);

    sf::RectangleShape frame(sf::Vector2f(field_select_width_, field_select_height_));
    frame.setPosition(field_select_x_, field_select_y_);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(200, 120, 200, 200));
    window_.draw(frame);

    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(24);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(210, 190, 255));
    title.setString(utf8(u8"ВЫБОР РАЗМЕРА ПОЛЯ"));
    title.setPosition(field_select_x_ + field_select_width_ / 2.f - 130.f, field_select_y_ + 20.f);
    window_.draw(title);
    

    sf::Text message;
    message.setFont(font_);
    message.setCharacterSize(18);
    message.setFillColor(sf::Color(190, 220, 255));
    message.setString(utf8(u8"Выберите размер игрового поля:"));
    message.setPosition(field_select_x_ + padding, field_select_y_ + 60.f);
    window_.draw(message);

    std::vector<std::string> size_options = {
        u8"выб_1 - 10×10 клеток",
        u8"выб_2 - 11×11 клеток",
        u8"выб_3 - 12×12 клеток",
        u8"выб_4 - 13×13 клеток",
        u8"выб_5 - 14×14 клеток",
    };

    float option_y = field_select_y_ + 95.f;
    for (const auto& option : size_options) {
        sf::Text option_text;
        option_text.setFont(font_);
        option_text.setCharacterSize(17);
        option_text.setFillColor(sf::Color(220, 200, 240));
        option_text.setString(utf8(option));
        option_text.setPosition(field_select_x_ + padding, option_y);
        window_.draw(option_text);
        option_y += 28.f;
    }

    sf::Text select_size;
    select_size.setFont(font_);
    select_size.setCharacterSize(17);
    select_size.setFillColor(sf::Color(190, 240, 220));
    select_size.setStyle(sf::Text::Bold);
    select_size.setString(utf8(u8"Выбранный размер: " + std::to_string(game.temp_field_size())));
    select_size.setPosition(field_select_x_ + padding, option_y + 10.f);
    window_.draw(select_size);


    sf::Text limits_info;
    limits_info.setFont(font_);
    limits_info.setCharacterSize(16);
    limits_info.setFillColor(sf::Color(220, 200, 240)); 
    limits_info.setString(utf8(u8"Ограничения:\n• размер клетки = 40px: до 13×13\n• размер клетки = 50px: только 10×10"));
    limits_info.setPosition(field_select_x_ + padding, option_y + 45.f);
    window_.draw(limits_info);

    sf::Text hint;
    hint.setFont(font_);
    hint.setCharacterSize(17);
    hint.setFillColor(sf::Color(180, 200, 230));
    hint.setString(utf8(u8"ДА - сохранить размер, НЕТ - отмена"));
    hint.setPosition(field_select_x_ + padding, field_select_y_ + field_select_height_ - 40.f);
    window_.draw(hint);
}

void GUIRenderer::RenderShipSizeSelection(const Game& game) {
    if (game.game_status() != GameStatus::SET_SIZES) return;
    ship_sizes_x_ = (window_.getSize().x - ship_sizes_width_) / 2.f;
    ship_sizes_y_ = (window_.getSize().y - ship_sizes_height_) / 2.f;
    const float padding = 30.f;
    
    sf::VertexArray gradient(sf::Quads, 4);
    sf::Color topColor(50, 25, 65);
    sf::Color bottomColor(90, 45, 95);
    gradient[0].position = sf::Vector2f(ship_sizes_x_, ship_sizes_y_);
    gradient[1].position = sf::Vector2f(ship_sizes_x_ + ship_sizes_width_, ship_sizes_y_);
    gradient[2].position = sf::Vector2f(ship_sizes_x_ + ship_sizes_width_, ship_sizes_y_ + ship_sizes_height_);
    gradient[3].position = sf::Vector2f(ship_sizes_x_, ship_sizes_y_ + ship_sizes_height_);
    gradient[0].color = topColor;
    gradient[1].color = topColor;
    gradient[2].color = bottomColor;
    gradient[3].color = bottomColor;
    window_.draw(gradient);
    
    sf::RectangleShape frame(sf::Vector2f(ship_sizes_width_, ship_sizes_height_));
    frame.setPosition(ship_sizes_x_, ship_sizes_y_);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(220, 140, 220, 200));
    window_.draw(frame);
    
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(24);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(250, 200, 250));
    title.setString(utf8(u8"СОЗДАНИЕ ФЛОТА"));
    title.setPosition(ship_sizes_x_ + ship_sizes_width_ / 2.f - 100.f, ship_sizes_y_ + 20.f);
    window_.draw(title);
    
    sf::Text message;
    message.setFont(font_);
    message.setCharacterSize(18);
    message.setFillColor(sf::Color(230, 210, 250));
    message.setString(utf8(u8"Добавьте корабли в свой флот:"));
    message.setPosition(ship_sizes_x_ + padding, ship_sizes_y_ + 60.f);
    window_.draw(message);
    
    std::vector<std::string> ship_options = {
        u8"выб_1 - корабль 1×1",
        u8"выб_2 - корабль 1×2",
        u8"выб_3 - корабль 1×3",
        u8"выб_4 - корабль 1×4",
        u8"выб_5 - стандартный флот (1x4,2x3,3x2,4x1)"
    };
    
    float option_y = ship_sizes_y_ + 95.f;
    for (const auto& option : ship_options) {
        sf::Text option_text;
        option_text.setFont(font_);
        option_text.setCharacterSize(16);
        option_text.setFillColor(sf::Color(220, 200, 240));
        option_text.setString(utf8(option));
        option_text.setPosition(ship_sizes_x_ + padding, option_y);
        window_.draw(option_text);
        option_y += 28.f;
    }
    
    sf::Text current_temp_fleet_title;
    current_temp_fleet_title.setFont(font_);
    current_temp_fleet_title.setCharacterSize(17);
    current_temp_fleet_title.setStyle(sf::Text::Bold);
    current_temp_fleet_title.setFillColor(sf::Color(190, 240, 220));
    current_temp_fleet_title.setString(utf8(u8"Созданный флот:"));
    current_temp_fleet_title.setPosition(ship_sizes_x_ + padding, option_y + 10.f);
    window_.draw(current_temp_fleet_title);
    
    sf::Text current_temp_fleet;
    current_temp_fleet.setFont(font_);
    current_temp_fleet.setCharacterSize(17);
    current_temp_fleet.setFillColor(sf::Color(200, 255, 230));
    current_temp_fleet.setString(utf8(game.fleet_spec_string(true)));
    current_temp_fleet.setPosition(ship_sizes_x_ + padding, option_y + 35.f);
    window_.draw(current_temp_fleet);
   
    sf::Text current_fleet_title;
    current_fleet_title.setFont(font_);
    current_fleet_title.setCharacterSize(17);
    current_fleet_title.setStyle(sf::Text::Bold);
    current_fleet_title.setFillColor(sf::Color(190, 240, 220));
    current_fleet_title.setString(utf8(u8"Текущий флот:"));
    current_fleet_title.setPosition(ship_sizes_x_ + padding, option_y + 75.f);
    window_.draw(current_fleet_title);
    
    sf::Text current_fleet;
    current_fleet.setFont(font_);
    current_fleet.setCharacterSize(17);
    current_fleet.setFillColor(sf::Color(200, 255, 230));
    current_fleet.setString(utf8(game.fleet_spec_string()));
    current_fleet.setPosition(ship_sizes_x_ + padding, option_y + 100.f);
    window_.draw(current_fleet);
    
    sf::Text hint;
    hint.setFont(font_);
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(190, 180, 220));
    hint.setString(utf8(u8"ДА - сохранить флот, НЕТ - отмена"));
    hint.setPosition(ship_sizes_x_ + padding, ship_sizes_y_ + ship_sizes_height_ - 35.f);
    window_.draw(hint);
}

void GUIRenderer::RenderStats(const Game& game) {
    if (!game.ShouldShowStats()) return;
    stats_x = ai_column_x_;
    stats_y = frame_y_ + column_height_ + ability_height_ + 20.f;
    const float padding = 10.f;
    
    sf::VertexArray gradient(sf::Quads, 4);
    sf::Color top_color(10, 15, 40);
    sf::Color bottom_color(25, 20, 60);
    gradient[0].position = sf::Vector2f(stats_x, stats_y);
    gradient[1].position = sf::Vector2f(stats_x + stats_width, stats_y);
    gradient[2].position = sf::Vector2f(stats_x + stats_width, stats_y + stats_height);
    gradient[3].position = sf::Vector2f(stats_x, stats_y + stats_height);
    gradient[0].color = top_color;
    gradient[1].color = top_color;
    gradient[2].color = bottom_color;
    gradient[3].color = bottom_color;
    window_.draw(gradient);
    
    sf::RectangleShape main_frame(sf::Vector2f(stats_width, stats_height));
    main_frame.setPosition(stats_x, stats_y);
    main_frame.setFillColor(sf::Color::Transparent);
    main_frame.setOutlineThickness(4.f);
    main_frame.setOutlineColor(sf::Color(80, 100, 255, 180));
    window_.draw(main_frame);
    
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(24);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color::White);
    title.setString(utf8(u8"СТАТИСТИКА"));
    title.setPosition(stats_x + stats_width / 2.f - 90.f, stats_y + 10.f);
    window_.draw(title);
    
    const std::string stats_text = game.statistics();
    std::vector<std::string> lines;
    std::stringstream ss(stats_text);
    std::string line;
    
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    float current_y = stats_y + 45.f;
    bool is_first_section = true;
   
    for (const auto& one_line : lines) {
        if (one_line.empty()) {
            continue;
        }
        sf::Text line_text;
        line_text.setFont(font_);
        line_text.setCharacterSize(16);
        line_text.setString(utf8(one_line));
        if (one_line.find(u8"СТАТИСТИКА") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setCharacterSize(20);
            line_text.setFillColor(sf::Color(255, 200, 100));
           
            if (!is_first_section) {
                current_y += 25.f;
            }
            is_first_section = false;
           
        } else if (one_line.find(u8"Победитель") != std::string::npos ||
                   one_line.find(u8"Счёт") != std::string::npos) {
            line_text.setStyle(sf::Text::Bold);
            line_text.setFillColor(sf::Color(100, 255, 200));
            current_y += 5.f;
           
        } else if (one_line.find('|') != std::string::npos) {
            line_text.setFillColor(sf::Color(180, 200, 255));
        } else {
            line_text.setFillColor(sf::Color(220, 240, 255));
        }
        line_text.setPosition(stats_x + padding, current_y);
        window_.draw(line_text);
       
        current_y += 22.f;
    }
}

void GUIRenderer::RenderGameTitle(int round) {
    title_x = (window_.getSize().x - title_width) / 2.f;

    sf::VertexArray gradient(sf::Quads, 4);
   
    gradient[0].position = sf::Vector2f(title_x, title_y);
    gradient[1].position = sf::Vector2f(title_x + title_width, title_y);
    gradient[0].color = sf::Color(20, 60, 140);
    gradient[1].color = sf::Color(20, 60, 140);
  
    gradient[2].position = sf::Vector2f(title_x + title_width, title_y + title_height);
    gradient[3].position = sf::Vector2f(title_x, title_y + title_height);
    gradient[2].color = sf::Color(0, 20, 70);
    gradient[3].color = sf::Color(0, 20, 70);
   
    window_.draw(gradient);
    sf::RectangleShape outline(sf::Vector2f(title_width, title_height));
    outline.setPosition(title_x, title_y);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineThickness(2);
    outline.setOutlineColor(sf::Color(80, 130, 210, 180));
    window_.draw(outline);

    std::vector<sf::Vector2f> background_bubbles = {
        sf::Vector2f(title_x + 15, title_y + 15),
        sf::Vector2f(title_x + 25, title_y + 43),
       
        sf::Vector2f(title_x + 472, title_y + 15),
        sf::Vector2f(title_x + 485, title_y + 35),
       
        sf::Vector2f(title_x + 115, title_y + 8),
        sf::Vector2f(title_x + 270, title_y + 9),
        sf::Vector2f(title_x + 380, title_y + 10),
   
        sf::Vector2f(title_x + 80, title_y + 25),
        sf::Vector2f(title_x + 160, title_y + 35),
       
        sf::Vector2f(title_x + 340, title_y + 30),
        sf::Vector2f(title_x + 420, title_y + 28),
       
        sf::Vector2f(title_x + 130, title_y + 49),
        sf::Vector2f(title_x + 270, title_y + 48),
        sf::Vector2f(title_x + 450, title_y + 49)
    };
   
    std::vector<float> background_bubble_sizes = {
        2.0f, 1.6f,
        1.8f, 2.2f,
        1.4f, 1.7f, 1.5f,
        1.9f, 1.3f,
        1.6f, 1.8f,
        2.1f, 1.5f, 1.7f
    };

    for (size_t i = 0; i < background_bubbles.size(); i++) {
        sf::CircleShape shadow(background_bubble_sizes[i]);
        shadow.setPosition(background_bubbles[i].x + 2.0f, background_bubbles[i].y + 2.0f);
        shadow.setFillColor(sf::Color(50, 100, 150, 165 * 0.8f));
        window_.draw(shadow);
        sf::CircleShape bubble(background_bubble_sizes[i]);
        bubble.setPosition(background_bubbles[i]);
       
        sf::VertexArray bubble_gradient(sf::TriangleFan, 5);
        bubble_gradient[0].position = sf::Vector2f(background_bubbles[i].x + background_bubble_sizes[i],
                                                 background_bubbles[i].y + background_bubble_sizes[i]);
        bubble_gradient[0].color = sf::Color(150, 200, 240, 165);
       
        bubble_gradient[1].position = sf::Vector2f(background_bubbles[i].x, background_bubbles[i].y);
        bubble_gradient[1].color = sf::Color(220, 240, 255, 165);
       
        bubble_gradient[2].position = sf::Vector2f(background_bubbles[i].x + background_bubble_sizes[i] * 2, background_bubbles[i].y);
        bubble_gradient[2].color = sf::Color(220, 240, 255, 165);
       
        bubble_gradient[3].position = sf::Vector2f(background_bubbles[i].x + background_bubble_sizes[i] * 2, background_bubbles[i].y + background_bubble_sizes[i] * 2);
        bubble_gradient[3].color = sf::Color(170, 210, 245, 165);
       
        bubble_gradient[4].position = sf::Vector2f(background_bubbles[i].x, background_bubbles[i].y + background_bubble_sizes[i] * 2);
        bubble_gradient[4].color = sf::Color(170, 210, 245, 165);
       
        window_.draw(bubble_gradient);
        sf::CircleShape bubble_outline(background_bubble_sizes[i]);
        bubble_outline.setPosition(background_bubbles[i]);
        bubble_outline.setFillColor(sf::Color::Transparent);
        bubble_outline.setOutlineThickness(1.2f);
        bubble_outline.setOutlineColor(sf::Color(240, 250, 255, 200));
        window_.draw(bubble_outline);
       
        sf::CircleShape highlight(background_bubble_sizes[i] * 0.4f);
        highlight.setPosition(background_bubbles[i].x + background_bubble_sizes[i] * 0.2f,
                             background_bubbles[i].y + background_bubble_sizes[i] * 0.15f);
        highlight.setFillColor(sf::Color(255, 255, 255, 220));
        window_.draw(highlight);
    }   
    
    std::string title_round = "МОРСКОЙ БОЙ - Раунд " + std::to_string(round);
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(32);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 250, 180));
    title.setOutlineThickness(2.0f);
    title.setOutlineColor(sf::Color(0, 15, 60, 200));
    title.setString(utf8(title_round));
   
    sf::FloatRect text_bounds = title.getLocalBounds();
    title.setPosition(
        title_x + (title_width - text_bounds.width) / 2.f,
        title_y + (title_height - text_bounds.height) / 2.f - 3.f
    );
   
    window_.draw(title);
}

void GUIRenderer::RenderScore(const Game& game) {
    std::string str_score = "счет: " + std::to_string(game.player_score()) + " - " + std::to_string(game.enemy_score());
    sf::Text score;
    score.setFont(font_);
    score.setCharacterSize(26);
    score.setStyle(sf::Text::Bold);
   
    sf::RectangleShape background(sf::Vector2f(180.f, 38.f));
    background.setPosition(player_pos_.x, 15.f);
    background.setFillColor(sf::Color(0, 0, 50));
    window_.draw(background);
    
    sf::RectangleShape frame(sf::Vector2f(180.f, 38.f));
    frame.setPosition(player_pos_.x, 15.f);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(100, 200, 255));
    window_.draw(frame);
    
    score.setFillColor(sf::Color(200, 230, 255));
    score.setOutlineThickness(1.f);
    score.setOutlineColor(sf::Color(0, 100, 200));
    score.setString(utf8(str_score));
    score.setPosition(player_pos_.x + 19.f, 17.f);
    window_.draw(score);
}

void GUIRenderer::RenderControlsLegend(const std::string& legend) {
    sf::Text text;
    text.setFont(font_);
    text.setCharacterSize(15);
    text.setFillColor(sf::Color(200, 220, 255));
    text.setStyle(sf::Text::Bold);
    
    std::vector<std::string> lines;
    std::stringstream ss(legend);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    float y = window_.getSize().y - 30 - lines.size() * 18;
    for (const auto& l : lines) {
        text.setString(utf8(l));
        text.setPosition(15, y);
        window_.draw(text);
        y += 23;
    }
}


void GUIRenderer::RenderError(){
    if (error_message_.empty()) return;

    sf::Text error_text;
    error_text.setFont(font_);
    error_text.setCharacterSize(19);
    error_text.setFillColor(sf::Color(255, 50, 50)); 
    error_text.setStyle(sf::Text::Bold);
    error_text.setString(utf8(error_message_));

    sf::FloatRect bounds = error_text.getLocalBounds();
    error_text.setPosition(player_pos_.x, window_.getSize().y - 160.f);

    sf::RectangleShape background(sf::Vector2f(bounds.width + 20.f, bounds.height + 10.f));
    background.setPosition(error_text.getPosition() - sf::Vector2f(3.f, 0.f));
    background.setFillColor(sf::Color(0, 0, 0, 200));
    background.setOutlineThickness(2.f);
    background.setOutlineColor(sf::Color(255, 80, 80));

    background.setFillColor(sf::Color(0, 0, 0));
    background.setOutlineColor(sf::Color(255, 80, 80));
    error_text.setFillColor(sf::Color(255, 80, 80));

    window_.draw(background);
    window_.draw(error_text);

    error_message_.clear();
}

void GUIRenderer::RenderFieldFrame(const PlayingField& field, const sf::Vector2f& position, const std::string& title) {
    const int w = field.x_size();
    const int h = field.y_size();
    
    sf::RectangleShape frame(sf::Vector2f(
        static_cast<float>(w * cell_spacing_ + 10),
        static_cast<float>(h * cell_spacing_ + 40)
    ));

    frame.setPosition(position - sf::Vector2f(5, 5));
    frame.setFillColor(sf::Color(0, 0, 0, 100));
    frame.setOutlineThickness(2);
    frame.setOutlineColor(sf::Color::White);
    window_.draw(frame);
    
    sf::Text title_text;
    title_text.setFont(font_);
    title_text.setCharacterSize(25);
    title_text.setFillColor(sf::Color::Cyan);
    title_text.setString(utf8(title));
    title_text.setPosition(position.x, position.y);
    window_.draw(title_text);
}

void GUIRenderer::RenderField(const PlayingField&  field, const std::string& title) {
    const bool is_enemy_field = (title == ai_name_);
    const sf::Vector2f& origin = is_enemy_field ? enemy_pos_ : player_pos_;
    RenderFieldFrame(field, origin, title);
    for (int y = 0; y < field.y_size(); ++y) {
        for (int x = 0; x < field.x_size(); ++x) {
            const sf::Vector2f pos(origin.x + x * cell_spacing_, origin.y + y * cell_spacing_ + 30.f);
            sf::Sprite water;
            water.setTexture(water_texture_);
            water.setPosition(pos);
            window_.draw(water);
            const Cell& vis = field.visible_cell(x, y);
            if (is_enemy_field) {
                if (field.IsScanned(x, y)) {
                    bool has_segment = field.IsShipCell(x, y);
                    DrawScanMark(pos + sf::Vector2f(cell_size_ * 0.5f, cell_size_ * 0.5f), has_segment);
                }
                if (vis.IsShip()) {
                    int ship_index = vis.ship_index();
                    int ship_size = 1;
                    std::tuple<int, SegmentState, Orientation> segment{0, SegmentState::INTACT, Orientation::HORIZONTAL};
                    bool fully_destroyed = false;
                    try {
                        const Ship& ship = field.ship(ship_index);
                        ship_size = ship.ship_size();
                        segment = ship.segment_state(x, y);
                        fully_destroyed = ship.IsDestroyed();
                    } catch (...) {}
                    RenderShipPart(pos, ship_size, segment, fully_destroyed);
                    DrawHitMark(pos + sf::Vector2f(cell_size_ * 0.5f, cell_size_ * 0.5f));
                } else if (vis.IsEmpty()) {
                    DrawMissMark(pos + sf::Vector2f(cell_size_ * 0.5f, cell_size_ * 0.5f));
                }
            } else {
                int ship_index = -1, segment_index = -1, ship_size = 1;
                SegmentState segment_state = SegmentState::INTACT;
                Orientation orientation = Orientation::HORIZONTAL;
                bool fully_destroyed = false;
                if (field.ship_info_at(x, y, ship_index, segment_index, ship_size, segment_state, orientation)) {
                    try {
                        fully_destroyed = field.ship(ship_index).IsDestroyed();
                    } catch (...) {}
                    RenderShipPart(pos, ship_size, std::make_tuple(segment_index, segment_state, orientation), fully_destroyed);
                }
                if (vis.IsShip()) {
                    DrawHitMark(pos + sf::Vector2f(cell_size_ * 0.5f, cell_size_ * 0.5f));
                } else if (vis.IsEmpty()) {
                    DrawMissMark(pos + sf::Vector2f(cell_size_ * 0.5f, cell_size_ * 0.5f));
                }
            }
        }
    }
}

void GUIRenderer::RenderCell(const PlayingField& /*field*/, CellState state, int ship_size,
                             const std::tuple<int, SegmentState, Orientation>& segment_info,
                             const sf::Vector2f& position, bool /*is_enemy_field*/) {
    sf::Sprite sprite;
    sprite.setTexture(water_texture_);
    sprite.setPosition(position);
    window_.draw(sprite);
    if (state == CellState::SHIP) {
        RenderShipPart(position, ship_size, segment_info, false);
    }
}

void GUIRenderer::RenderShipPart(const sf::Vector2f& position, int ship_size,
                                const std::tuple<int, SegmentState, Orientation>& segmentInfo,
                                bool fullyDestroyed) {
    const int segment_index = std::get<0>(segmentInfo);
    const SegmentState state = std::get<1>(segmentInfo);
    const Orientation orientation = std::get<2>(segmentInfo);
    sf::Sprite sprite;
    if (fullyDestroyed && !wreckage_textures_.empty()) {
        const int s = std::clamp(ship_size, 1, 4);
        const int ci = CellIdx();
        const int index = (s - 1) * 3 + ci;
        if (index >= 0 && index < static_cast<int>(wreckage_textures_.size())) {
            sprite.setTexture(wreckage_textures_[index]);
            sprite.setPosition(position);
            window_.draw(sprite);
            return;
        }
    }
    const int s = std::clamp(ship_size, 1, 4);
    const int ci = CellIdx();
    const bool is_damaged = (state == SegmentState::DAMAGED || state == SegmentState::DESTROYED);
    const auto& set = ship_texture_[s][ci];
    const auto& vec = is_damaged ? set.damaged : set.intact;
    int index = std::clamp(segment_index, 0, s - 1);
    if (index >= static_cast<int>(vec.size())) return;
    sprite.setTexture(vec[index]);
    sprite.setPosition(position);
    if (orientation == Orientation::VERTICAL) {
        sprite.setRotation(90.f);
        sprite.move(static_cast<float>(cell_size_), 0.f);
    }
    window_.draw(sprite);
}

void GUIRenderer::RenderCursor(const Game& game) {
    const int cx = game.cursor_x();
    const int cy = game.cursor_y();
   
    sf::Vector2f cursor_pos;
    if (game.game_status() == GameStatus::PLACING_SHIPS || game.game_status() == GameStatus::SET_FIELD 
        || game.game_status() == GameStatus::SET_SIZES ) {
        cursor_pos = sf::Vector2f(player_pos_.x + cx * cell_spacing_, player_pos_.y + cy * cell_spacing_ + 30.f);
    
        if (!error_message_.empty() && game.placement_mode() == PlacementMode::MANUAL) {
            RenderErrorShip(game, cx, cy);
            return ;
        }
    } else {
        cursor_pos = sf::Vector2f(enemy_pos_.x + cx * cell_spacing_, enemy_pos_.y + cy * cell_spacing_ + 30.f);
    }
   
    sf::RectangleShape cursor(sf::Vector2f(static_cast<float>(cell_size_), static_cast<float>(cell_size_)));
    cursor.setPosition(cursor_pos);
    cursor.setFillColor(sf::Color(255, 255, 0, 64));
    cursor.setOutlineThickness(2.f);
    cursor.setOutlineColor(sf::Color::Yellow);
    window_.draw(cursor);
}

void GUIRenderer::RenderGameStatus(const Game& game) {
    GameStatus status = game.game_status();
    const char* text = "";
    sf::Color color = sf::Color::White;
    switch (status) {
        case GameStatus::PLACING_SHIPS:
            text = game.placement_mode() == PlacementMode::AUTO ? u8"Расстановка кораблей (Авто)" : u8"Расстановка кораблей (Ручная)";
            color = sf::Color::Cyan;
            break;
        case GameStatus::PLAYER_TURN: text = u8"Ваш ход"; color = sf::Color::Green; break;
        case GameStatus::ENEMY_TURN: text = u8"Ход противника"; color = sf::Color::Yellow; break;
        case GameStatus::PLAYER_WON: text = u8"Вы победили!"; color = sf::Color(0, 255, 0);
            break;
        case GameStatus::ENEMY_WON: text = u8"Противник победил"; color = sf::Color::Red; break;
        case GameStatus::PAUSED: text = u8"Пауза"; color = sf::Color(255, 165, 0);
            break;
        case GameStatus::GAME_OVER: text = u8"Игра завершена"; color = sf::Color::Magenta; break;
        case GameStatus::SET_FIELD: text = u8"Настройка поля"; color = sf::Color(216, 191, 216);
            break;
        case GameStatus::SET_SIZES: text = u8"Настройка флота"; color = sf::Color(173, 216, 230);
            break;
        case GameStatus::ASK_SAVE: text = u8"Сохранение игры"; color = sf::Color(255, 69, 0);
            break;
        case GameStatus::SETTING_SHIPS: text = u8"Изменение кораблей"; color = sf::Color(210, 180, 140);
            break;
        case GameStatus::ASK_EXIT: text = u8"Подтверждение выхода"; color = sf::Color(255, 215, 0);
            break;
        case GameStatus::WAITING_NEXT_ROUND: text = u8"Ожидание следующего раунда"; color = sf::Color(230, 210, 255);
            break;
        case GameStatus::SELECT_LOAD_SLOT: text = u8"Выбор слота загрузки"; color = sf::Color(144, 238, 144);
            break;
        case GameStatus::SELECT_SAVE_SLOT: text = u8"Выбор слота сохранения"; color = sf::Color(255, 192, 203);
            break;
        default: text = u8"Неизвестный статус"; color = sf::Color::White; break;
    }
    sf::Text status_display;
    status_display.setFont(font_);
    status_display.setCharacterSize(22);
    status_display.setFillColor(color);
    status_display.setString(utf8(text));
    status_display.setPosition(player_pos_.x, player_pos_.y + grid_height_ + 10.f);
    window_.draw(status_display);
}



void GUIRenderer::RenderErrorShip(const Game& game, int start_x, int start_y) {
    auto [ship_size, orientation] = game.current_ship_info();
    const auto player_field = game.player_field();
    const sf::Vector2f& origin = player_pos_;
        
    for (int i = 0; i < ship_size; ++i) {
        int x = (orientation == Orientation::HORIZONTAL) ? start_x + i : start_x;
        int y = (orientation == Orientation::VERTICAL) ? start_y + i : start_y;
            
        if (x >= 0 && x < player_field.x_size() && y >= 0 && y < player_field.y_size()) {
            const sf::Vector2f pos(origin.x + x * cell_spacing_, origin.y + y * cell_spacing_ + 30.f);
                
            sf::RectangleShape error_cell(sf::Vector2f(static_cast<float>(cell_size_), static_cast<float>(cell_size_)));
            error_cell.setPosition(pos);
            error_cell.setFillColor(sf::Color(255, 0, 0, 80));  
            error_cell.setOutlineThickness(2.f);      
            error_cell.setOutlineColor(sf::Color::Red);        
            window_.draw(error_cell);
        }
    }
}



bool GUIRenderer::LoadResources() {
    std::vector<std::string> font_paths = {
        "assets/fonts/ARIAL.TTF",
        "assets/Fonts/DEJAVUSANS.TTF"
    };
    bool font_loaded = false;
    for (const auto& path : font_paths) {
        if (font_.loadFromFile(path)) {
            font_loaded = true;
            break;
        }
    }
    if (!font_loaded) {
        std::cerr << "! Предупреждение: шрифт не загружен. Текст может не отображаться.\n";
    }
    if (!LoadShipTextures()) {
        std::cerr << "! Критическая ошибка: не удалось загрузить текстуры кораблей\n";
        return false;
    }
    std::string water_path;
    switch (cell_size_) {
        case 30: water_path = "resources/water/water_30.png"; break;
        case 40: water_path = "resources/water/water_40.png"; break;
        case 50: water_path = "resources/water/water_50.png"; break;
        default: water_path = "resources/water/water_40.png";
    }
    if (!water_texture_.loadFromFile(water_path)) {
        std::cerr << "! Текстура воды не загружена: " << water_path << ", используется fallback\n";
        water_texture_.create(cell_size_, cell_size_);
        sf::Image water_image;
        water_image.create(cell_size_, cell_size_, sf::Color(30, 130, 230, 200));
        water_texture_.update(water_image);
    }
    if (!LoadWreckageTextures()) {
        std::cerr << "! Предупреждение: не все текстуры обломков загружены. Будет использован вид Damaged.\n";
    }
    return true;
}

static bool LoadTextureOrLog(sf::Texture& t, const std::string& path) {
    if (!t.loadFromFile(path)) {
        std::cerr << "Не удалось загрузить текстуру: " << path << "\n";
        return false;
    }
    return true;
}

bool GUIRenderer::LoadShipTextures() {
    for (auto& by_size : ship_texture_) {
        for (auto& set : by_size) {
            set.intact.clear();
            set.damaged.clear();
        }
    }
    auto loadFor = [&](int size, int px) -> bool {
        int ci = (px == 30 ? 0 : (px == 40 ? 1 : 2));
        auto& set = ship_texture_[size][ci];
        auto push = [&](std::vector<sf::Texture>& vec, const std::string& path) -> bool {
            sf::Texture t;
            if (!LoadTextureOrLog(t, path)) return false;
            vec.push_back(std::move(t));
            return true;
        };
        if (size == 1) {
            if (!push(set.intact, "resources/ship_1/1 ship " + std::to_string(px) + ".png")) return false;
            if (!push(set.damaged, "resources/ship_1/1 ship " + std::to_string(px) + " damaged.png")) return false;
        } else if (size == 2) {
            if (!push(set.intact, "resources/ship_2/2 ship_1 " + std::to_string(px) + ".png")) return false;
            if (!push(set.intact, "resources/ship_2/2 ship_2 " + std::to_string(px) + ".png")) return false;
            if (!push(set.damaged, "resources/ship_2/2 ship " + std::to_string(px) + " damaged_1.png")) return false;
            if (!push(set.damaged, "resources/ship_2/2 ship " + std::to_string(px) + " damaged_2.png")) return false;
        } else if (size == 3) {
            if (!push(set.intact, "resources/ship_3/3 ship_1 " + std::to_string(px) + ".png")) return false;
            if (!push(set.intact, "resources/ship_3/3 ship_2 " + std::to_string(px) + ".png")) return false;
            if (!push(set.intact, "resources/ship_3/3 ship_3 " + std::to_string(px) + ".png")) return false;
            if (!push(set.damaged, "resources/ship_3/3 ship " + std::to_string(px) + " damaged_1.png")) return false;
            if (!push(set.damaged, "resources/ship_3/3 ship " + std::to_string(px) + " damaged_2.png")) return false;
            if (!push(set.damaged, "resources/ship_3/3 ship " + std::to_string(px) + " damaged_3.png")) return false;
        } else if (size == 4) {
            if (!push(set.intact, "resources/ship_4/4 ship_1 " + std::to_string(px) + ".png")) return false;
            if (!push(set.intact, "resources/ship_4/4 ship_2 " + std::to_string(px) + ".png")) return false;
            if (!push(set.intact, "resources/ship_4/4 ship_3 " + std::to_string(px) + ".png")) return false;
            if (!push(set.intact, "resources/ship_4/4 ship_4 " + std::to_string(px) + ".png")) return false;
            if (!push(set.damaged, "resources/ship_4/4 ship " + std::to_string(px) + " damaged_1.png")) return false;
            if (!push(set.damaged, "resources/ship_4/4 ship " + std::to_string(px) + " damaged_2.png")) return false;
            if (!push(set.damaged, "resources/ship_4/4 ship " + std::to_string(px) + " damaged_3.png")) return false;
            if (!push(set.damaged, "resources/ship_4/4 ship " + std::to_string(px) + " damaged_4.png")) return false;
        }
        return true;
    };
    bool ok = true;
    ok = ok && loadFor(1, 30) && loadFor(1, 40) && loadFor(1, 50);
    ok = ok && loadFor(2, 30) && loadFor(2, 40) && loadFor(2, 50);
    ok = ok && loadFor(3, 30) && loadFor(3, 40) && loadFor(3, 50);
    ok = ok && loadFor(4, 30) && loadFor(4, 40) && loadFor(4, 50);
    return ok;
}

bool GUIRenderer::LoadWreckageTextures() {
    wreckage_textures_.clear();
    const char* patterns[4][3] = {
        {"resources/explosions/ship_planks_1 30.png", "resources/explosions/ship_planks_1 40.png", "resources/explosions/ship_planks_1 50.png"},
        {"resources/explosions/ship_planks_2 30.png", "resources/explosions/ship_planks_2 40.png", "resources/explosions/ship_planks_2 50.png"},
        {"resources/explosions/ship_planks_3 30.png", "resources/explosions/ship_planks_3 40.png", "resources/explosions/ship_planks_3 50.png"},
        {"resources/explosions/ship_planks_4 30.png", "resources/explosions/ship_planks_4 40.png", "resources/explosions/ship_planks_4 50.png"}
    };
    bool ok = true;
    for (int sz = 0; sz < 4; ++sz) {
        for (int s = 0; s < 3; ++s) {
            sf::Texture tex;
            if (!tex.loadFromFile(patterns[sz][s])) {
                std::cerr << "Не удалось загрузить текстуру обломков: " << patterns[sz][s] << "\n";
                ok = false;
                tex.create(cell_size_, cell_size_);
                sf::Image img; img.create(cell_size_, cell_size_, sf::Color::Transparent);
                tex.update(img);
            }
            wreckage_textures_.push_back(std::move(tex));
        }
    }
    return ok;
}

void GUIRenderer::OnAttackResult(const AttackResult& result) {
    OnAttackResult(result, true);
}

void GUIRenderer::OnAttackResult(const AttackResult& result, bool on_enemy_field) {
    if (result.hit > 0) sound_manager_.PlayExplosion();
    else if (result.hit == 0) sound_manager_.PlayWaterSplash();
   
    std::string person = on_enemy_field ? "Вы" : ai_name_;
    std::string msg = person;
   
    if (result.hit < 0) {
        msg += u8": недействительный выстрел (" + std::to_string(result.x) + ", " + std::to_string(result.y) + ")";
    } else if (result.hit == 2) {
        msg += u8": уничтожение корабля!";
    } else if (result.hit == 1) {
        msg += u8": повреждение сегмента (" + std::to_string(result.x) + ", " + std::to_string(result.y) + ")";
    } else {
        msg += u8": промах (" + std::to_string(result.x) + ", " + std::to_string(result.y) + ")";
    }
   
    AddMessage(msg);
}

void GUIRenderer::OnAbilityResult(const AbilityResult& result) {
    sound_manager_.PlayAbilitySound(result.ability_name);
   
    std::string msg;
    if (result.ability_name == "Double Damage") {
        msg = u8" Урон x2 для атаки" + std::to_string(result.x) + ", " + std::to_string(result.y) + ")";
    } else if (result.ability_name == "Scanner") {
        msg = u8" Скан области вокруг (" + std::to_string(result.x) + ", " + std::to_string(result.y) + ")";
    } else if (result.ability_name == "Shelling") {
        msg = u8" Обстрел по координатам (" + std::to_string(result.x) + ", " + std::to_string(result.y) + ")";
    }
   
    AddMessage(msg);
}

void GUIRenderer::ShowShotBanner(const std::string& text, bool on_enemy_field) {
    banner_.text = text;
    banner_.on_enemy_field = on_enemy_field;
    banner_.active = true;
    banner_.timer.restart();
    AddMessage(text);
}

void GUIRenderer::RenderBanners() {
    if (!banner_.active) return;
    float t = banner_.timer.getElapsedTime().asSeconds();
    if (t > banner_duration_sec_) { banner_.active = false; return; }
    const sf::Vector2f& origin = banner_.on_enemy_field ? enemy_pos_ : player_pos_;
    sf::Text text;
    text.setFont(font_);
    text.setCharacterSize(18);
    text.setStyle(sf::Text::Bold);
    text.setFillColor(sf::Color::Yellow);
    text.setString(utf8(banner_.text));
    text.setPosition(origin.x, origin.y - 35.f);
    window_.draw(text);
}

int GUIRenderer::CellIdx() const { 
    return (cell_size_ == 30) ? 0 : (cell_size_ == 40 ? 1 : 2); 
}

void GUIRenderer::DrawHitMark(const sf::Vector2f& center) {
    sf::Vertex lines[4] = {
        sf::Vertex(center + sf::Vector2f(-cell_size_ * 0.4f, -cell_size_ * 0.4f), sf::Color::Red),
        sf::Vertex(center + sf::Vector2f(cell_size_ * 0.4f, cell_size_ * 0.4f), sf::Color::Red),
        sf::Vertex(center + sf::Vector2f(-cell_size_ * 0.4f, cell_size_ * 0.4f), sf::Color::Red),
        sf::Vertex(center + sf::Vector2f(cell_size_ * 0.4f, -cell_size_ * 0.4f), sf::Color::Red),
    };
    window_.draw(lines, 2, sf::Lines);
    window_.draw(lines + 2, 2, sf::Lines);
}

void GUIRenderer::DrawMissMark(const sf::Vector2f& center) {
    sf::CircleShape dot(cell_size_ * 0.1f);
    dot.setFillColor(sf::Color::White);
    dot.setOrigin(dot.getRadius(), dot.getRadius());
    dot.setPosition(center);
    window_.draw(dot);
}

void GUIRenderer::DrawScanMark(const sf::Vector2f& center, bool positive) {
    if (positive) {
        sf::ConvexShape diamond(4);
        float r = cell_size_ * 0.22f;
        diamond.setPoint(0, center + sf::Vector2f(0.f, -r));
        diamond.setPoint(1, center + sf::Vector2f(r, 0.f));
        diamond.setPoint(2, center + sf::Vector2f(0.f, r));
        diamond.setPoint(3, center + sf::Vector2f(-r, 0.f));
        diamond.setFillColor(sf::Color(255, 215, 0, 210));
        window_.draw(diamond);
    } else {
        sf::CircleShape dot(cell_size_ * 0.08f);
        dot.setFillColor(sf::Color(200, 200, 200, 210));
        dot.setOrigin(dot.getRadius(), dot.getRadius());
        dot.setPosition(center);
        window_.draw(dot);
    }
}

void GUIRenderer::ShowMessage(const std::string& message) {
    AddMessage(message);
}

void GUIRenderer::AddMessage(const std::string& message) {
    if (message.find(ai_name_) != std::string::npos){
        ai_log_.push_back(message);
        if (ai_log_.size() > max_log_lines_) {
            ai_log_.erase(ai_log_.begin());
        }
    } else if (message.find("ошибк") != std::string::npos || 
               message.find("Ошибк") != std::string::npos) {
        error_message_ = message;
    } else if (message.find("cпособност") != std::string::npos || message.find("Способност") != std::string::npos){
        ability_log_.push_back(message);
        if (ability_log_.size() > max_log_lines_) {
            ability_log_.erase(ability_log_.begin());
        }
    } else if (message.find("загружен") != std::string::npos || message.find("Загружен") != std::string::npos
    || message.find("сохранен") != std::string::npos || message.find("Cохранен") != std::string::npos){
        save_load_message_ = message;
        show_save_load_message_ += 4;
    } else {
        player_log_.push_back(message);
        if (player_log_.size() > max_log_lines_) {
            player_log_.erase(player_log_.begin());
        }
   
    }
}

std::vector<std::string> GUIRenderer::wrapText(const std::string& text, float max_width, unsigned int character_size) {
    std::vector<std::string> lines;
    if (text.empty() || text.find_first_not_of(' ') == std::string::npos) {
        return lines;
    }
   
    sf::Text temp_text;
    temp_text.setFont(font_);
    temp_text.setCharacterSize(character_size);
   
    std::string current_line;
    std::string current_word;
   
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
       
        if (c == ' ' || c == '\t') {
            if (!current_word.empty()) {
                std::string test_line = current_line + (current_line.empty() ? "" : " ") + current_word;
                temp_text.setString(utf8(test_line));
               
                if (temp_text.getLocalBounds().width <= max_width) {
                    current_line = test_line;
                } else {
                    if (!current_line.empty()) {
                        lines.push_back(current_line);
                    }
                    current_line = current_word;
                }
                current_word.clear();
            }
        } else if (c == '\n') {
            if (!current_word.empty()) {
                current_line = current_line + (current_line.empty() ? "" : " ") + current_word;
                current_word.clear();
            }
            if (!current_line.empty()) {
                lines.push_back(current_line);
                current_line.clear();
            }
        } else {
            current_word += c;
        }
    }
   
    if (!current_word.empty()) {
        std::string test_line = current_line + (current_line.empty() ? "" : " ") + current_word;
        temp_text.setString(utf8(test_line));
       
        if (temp_text.getLocalBounds().width <= max_width) {
            current_line = test_line;
        } else {
            if (!current_line.empty()) {
                lines.push_back(current_line);
            }
            current_line = current_word;
        }
    }
   
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
   
    return lines;
} 