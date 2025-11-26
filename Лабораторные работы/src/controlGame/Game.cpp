#include "Game.h"
#include "abilities/AbilityException.h"
#include <thread>
#include <iomanip>
#include <iostream>
#include <random>
#include "core/Player.h"
#include "ShipCoordinateExceptions.h"


Game::Game(GameSettings new_settings) : human_name_(new_settings.player_name()), settings_(std::move(new_settings)) {
    try {
        Initialize();
    } catch (const std::exception& e) {
        std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: Не удалось инициализировать игру\n";
        std::cerr << "Причина: " << e.what() << std::endl;
        std::cerr << "Программа будет завершена." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

Game::~Game() { CleanUp();}

void Game::CreateShipManager(){
    std::vector<int> fleet;
    fleet = settings_.fleet_spec();
    if (fleet.empty()) fleet = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    ship_manager_ = std::make_shared<ShipManager>(fleet.size(), fleet);
}


void Game::Initialize(){
    const int size = settings_.field_size();
    CreateShipManager();
    human_player_ = std::make_unique<Player>(human_name_, PlayerType::HUMAN, ship_manager_, std::move(ability_manager_), size, size);
    ai_player_ = std::make_unique<Player>("AI", PlayerType::AI, ship_manager_, nullptr, size, size);
    set_players(std::move(human_player_), std::move(ai_player_));
}

void Game::MoveAIShips() {
    if (!ai_player_->PlaceShipsRandomly()) {
        settings_.ResetFieldAndShipSize();
        Initialize();
        throw ImpossibleFleetException();
    }
}

void Game::MoveRandomShips() {
    if (!human_player_->PlaceShipsRandomly()) {
        settings_.ResetFieldAndShipSize();
        Initialize();
        throw ImpossibleFleetException();
    }
}

std::pair<int, Orientation> Game::current_ship_info() const {
    int ship_size = current_ship_size();
    Orientation orientation = ship_orientation();
    return std::make_pair(ship_size, orientation);
}


int Game::current_ship_size() const {
    if (human_player_->field().HasShipsToReplace()) {
        return human_player_->field().removed_ship_size();
    }
    else {
        int ships_placed = human_player_->field().count();
        return ship_manager_->ship_size(ships_placed);
    }
}


void Game::MoveShip(int x, int y, Orientation orientation) {
    int ship_size = current_ship_size();
    human_player_->field_for_modification().PlaceShip(x, y, ship_size, orientation);
    if (!CanPlaceShip()) {
        show_ships_info_ = false;
        MoveAIShips();
        current_state_.set_game_status(GameStatus::PLAYER_TURN);
    }
}


AttackResult Game::MakeAIMove() {
    AttackResult out{ -1, -1, -1 };

    const PlayingField& human_field = human_player_->field();
    const int W = human_field.x_size();
    const int H = human_field.y_size();

    // 1) цели для добивания: клетки, где сегмент в состоянии DamageD
    std::vector<std::pair<int,int>> damaged_targets;

    // 2) фронтир: UNKNOWN-соседи рядом с уже найденными сегментами
    std::vector<std::pair<int,int>> frontier;

    // 3) просто неизвестные клетки (fallback)
    std::vector<std::pair<int,int>> unknown;

    damaged_targets.reserve(W*H/8);
    frontier.reserve(W*H/2);
    unknown.reserve(W*H);

    auto inb = [&](int x, int y){ return x>=0 && y>=0 && x<W && y<H; };

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            const Cell& vis = human_field.visible_cell(x, y);

            if (vis.IsShip()) {
                // разрешено смотреть истинное состояние ТОЛЬКО по уже «открытым» клеткам
                int ship_index, segment_index, ship_size;
                SegmentState segment_state;
                Orientation orient;
                if (human_field.ship_info_at(x, y, ship_index, segment_index, ship_size, segment_state, orient)) {
                    if (segment_state == SegmentState::DAMAGED) {
                        damaged_targets.emplace_back(x, y); // приоритет №1 — добиваем
                    }
                }

                // наращиваем фронтир вокруг любых найденных сегментов
                if (inb(x+1,y) && human_field.visible_cell(x+1,y).IsUnknown()) frontier.emplace_back(x+1,y);
                if (inb(x-1,y) && human_field.visible_cell(x-1,y).IsUnknown()) frontier.emplace_back(x-1,y);
                if (inb(x,y+1) && human_field.visible_cell(x,y+1).IsUnknown()) frontier.emplace_back(x,y+1);
                if (inb(x,y-1) && human_field.visible_cell(x,y-1).IsUnknown()) frontier.emplace_back(x,y-1);
            } else if (vis.IsUnknown()) {
                unknown.emplace_back(x, y);
            }
        }
    }

    // выбираем цель по приоритетам
    int tx = -1, ty = -1;
    std::random_device rd;
    std::mt19937 gen(rd());

    if (!damaged_targets.empty()) {
        std::uniform_int_distribution<> pick(0, static_cast<int>(damaged_targets.size()) - 1);
        std::tie(tx, ty) = damaged_targets[pick(gen)];
    } else if (!frontier.empty()) {
        std::uniform_int_distribution<> pick(0, static_cast<int>(frontier.size()) - 1);
        std::tie(tx, ty) = frontier[pick(gen)];
    } else if (!unknown.empty()) {
        std::uniform_int_distribution<> pick(0, static_cast<int>(unknown.size()) - 1);
        std::tie(tx, ty) = unknown[pick(gen)];
    } else {
        // целей нет — всё открыто
        out.hit = -1; out.x = 0; out.y = 0;
        return out;
    }

    // совершаем выстрел (правила поля уже позволяют стрелять повторно по DamageD)
    int res = ai_player_->MakeMove(human_player_, tx, ty);
    // res: -1 повтор/некорректно; 0 — мимо; 1 — попадание (или добили сегмент, но корабль ещё жив); 2 — потопление

    out.x   = tx;
    out.y   = ty;
    out.hit = res;

    UpdateTotalStats();
    UpdateScore();        
    CheckWinCondition();

    // передаём ход игроку только после валидного выстрела ИИ и если игра не завершилась
    if (res >= 0 && current_state_.game_status() != GameStatus::PLAYER_WON &&
        current_state_.game_status() != GameStatus::ENEMY_WON) {
        current_state_.set_game_status(GameStatus::PLAYER_TURN);
    }

    return out;
}

AttackResult Game::AttackShipAt(int x, int y) {
    int result = human_player_->MakeMove(ai_player_, x, y);
    UpdateTotalStats();
    UpdateScore();
    current_state_.set_game_status(GameStatus::ENEMY_TURN);
    CheckWinCondition();
    return {result, x, y};
}

AttackResult Game::AttackShip() {
    int cx = current_state_.cursor_x();
    int cy = current_state_.cursor_y();
    if (cx >= 0 && cy >= 0) {
        return AttackShipAt(cx, cy);
    }
    return {-1, -1, -1};
}

AbilityResult Game::UseAbility(int x, int y) {
    auto coordinates = human_player_->UseAbility(x, y);
    auto ability_name = ability_manager_->ability_name(); 
    if (coordinates.first != -1) {
        UpdateTotalStats();
        UpdateScore();
        current_state_.set_game_status(GameStatus::ENEMY_TURN);
    }
    return {ability_name, coordinates.first, coordinates.second};
}

void Game::UpdateScore() {
    PlayerStats player_stats = current_state_.player_stats();
    PlayerStats enemy_stats  = current_state_.enemy_stats();

    player_stats.destroyed = human_player_->destroyed_ships();
    player_stats.hits      = human_player_->hit_count();
    player_stats.shots     = human_player_->all_shots();
    player_stats.accuracy  = human_player_->accuracy();
    player_stats.remaining = human_player_->remaining_ships();

    enemy_stats.destroyed  = ai_player_->destroyed_ships();
    enemy_stats.hits       = ai_player_->hit_count();
    enemy_stats.shots      = ai_player_->all_shots();
    enemy_stats.accuracy   = ai_player_->accuracy();
    enemy_stats.remaining  = ai_player_->remaining_ships();

    current_state_.set_player_stats(player_stats);
    current_state_.set_enemy_stats(enemy_stats);
}

void Game::UpdateTotalStats() {
    TotalPlayerStats total_player_stats = current_state_.total_player_stats();
    total_player_stats.total_hits  += human_player_->hit_count() - current_state_.player_stats().hits;
    total_player_stats.total_shots += human_player_->all_shots() - current_state_.player_stats().shots;

    TotalPlayerStats total_enemy_stats = current_state_.total_enemy_stats();
    total_enemy_stats.total_hits  += ai_player_->hit_count() - current_state_.enemy_stats().hits;
    total_enemy_stats.total_shots += ai_player_->all_shots() - current_state_.enemy_stats().shots;

    current_state_.set_total_player_stats(total_player_stats);   
    current_state_.set_total_enemy_stats(total_enemy_stats);    
}

void Game::CheckWinCondition() {
    if (ai_player_->IsAllShipsDestroyed()) {
        current_state_.set_game_status(GameStatus::ENEMY_WON);
        current_state_.set_round_result(0); 
        EndRound(); 
    }
    else if (human_player_->IsAllShipsDestroyed()) {
        current_state_.set_game_status(GameStatus::PLAYER_WON);
        current_state_.set_round_result(1);
        EndRound();
    }
}



std::string Game::statistics() const {
    std::ostringstream ss;

    PlayerStats round_player = current_state_.player_stats();
    PlayerStats round_enemy = current_state_.enemy_stats();
    TotalPlayerStats total_player = current_state_.total_player_stats();
    TotalPlayerStats total_enemy = current_state_.total_enemy_stats();

    ss << u8"СТАТИСТИКА РАУНДА " << current_state_.round_number() << "\n";
    ss << u8"Победитель: ";
    
    std::string winner;
    int round_result = current_state_.round_result();
    switch (round_result) {
        case 1:  winner = "ВЫ"; break;
        case 0:  winner = "ПРОТИВНИК"; break;
        case -1: winner = "НИЧЬЯ"; break;
        default: winner = u8"—"; break;
    }
    ss << winner << "\n\n";

    const std::string enemy_name = round_enemy.name;

    auto PrintLine = [&](const std::string& name, const PlayerStats& p) {
        ss << std::left << std::setw(6) << name
           << u8"| Попадания: " << std::setw(3) << p.hits
           << u8"| Выстрелы: " << std::setw(3) << p.shots
           << u8"| Точность: " << std::setw(5)
           << (p.shots > 0 ? std::to_string((p.hits * 100) / p.shots) + "%" : "-")
           << u8"| Уничтожил: " << std::setw(2) << p.destroyed
           << u8"| Осталось: " << p.remaining << "\n";
    };

    PrintLine("Вы", round_player);
    PrintLine(enemy_name, round_enemy);

    ss << "\n";
    ss << u8"ОБЩАЯ СТАТИСТИКА\n";
    ss << u8"Счёт: " << total_player.count_won << u8" : "
       << total_enemy.count_won <<"\n\n";

    auto PrintTotalLine = [&](const std::string& name, const TotalPlayerStats& p) {
        ss << std::left << std::setw(6) << name
           << u8"| Попадания: " << std::setw(3) << p.total_hits
           << u8"| Выстрелы: " << std::setw(3) << p.total_shots
           << u8"| Точность: " << std::setw(5)
           << (p.total_shots > 0 ? std::to_string((p.total_hits * 100) / p.total_shots) + "%" : "-")
           << u8"| Победы: " << std::setw(2) << p.count_won
           << u8"| Раунды: " << p.rounds << "\n";
    };

    PrintTotalLine("Вы", total_player);
    PrintTotalLine(enemy_name, total_enemy);

    return ss.str();
}



std::vector<ShipDisplayInfo> Game::human_player_ships_info() const {
    std::vector<ShipDisplayInfo> result;
    const PlayingField field = human_player_->field();

    for (int i = 0; i < ship_manager_->ship_count(); i++) {
        ShipDisplayInfo info;
        info.number = i + 1;
        
        bool found = false;
        for (int j = 0; j < field.count(); j++) {
            try {
                const Ship& ship = field.ship(j);
                if (ship.ship_number() == i) { 
                    info.is_placed = true;
                    info.start_pos = ship.start_position();
                    info.orientation = ship.orientation();
                    info.size = ship.ship_size(); 
                    found = true;
                    break;
                }
            } catch (...) {
            }
        }
        
        if (!found) {
            info.is_placed = false;
            info.start_pos = Position{-1, -1};
            info.orientation = Orientation::HORIZONTAL;
            info.size = ship_manager_->ship_size(i); 
        }
        
        result.push_back(info);
    }
    
    return result;
}



bool Game::CanSaveGame() const { return current_state_.CanSave(); }
bool Game::CanLoadGame() const { return current_state_.CanLoad(); }

bool Game::IsNotRunning() {
    GameStatus status = current_state_.game_status();
    return status == GameStatus::GAME_OVER || status == GameStatus::PAUSED    
        || status == GameStatus::PLAYER_WON || status == GameStatus::ENEMY_WON;
}


std::string Game::ShowAbility() const{
    return ability_manager_->PeekNextAbility();  
}

void Game::set_players(std::unique_ptr<Player> human, std::unique_ptr<Player> ai) {
    human_player_ = std::move(human);
    ai_player_    = std::move(ai);

    ship_manager_ = human_player_->ship_manager();

    ability_manager_ = std::make_unique<AbilityManager>(ai_player_->field_for_modification(), human_player_->field_for_modification());
    human_player_->set_ability_manager(std::move(ability_manager_)); 

    current_state_.set_game_status(GameStatus::PLACING_SHIPS);
    current_state_.set_cursor(0, 0);

    auto ps = current_state_.player_stats();
    ps.name = human_player_->name();
    current_state_.set_player_stats(ps);

    auto es = current_state_.enemy_stats();
    es.name = ai_player_->name();
    current_state_.set_enemy_stats(es);

    auto tps = current_state_.total_player_stats();
    tps.name = human_player_->name();
    current_state_.set_total_player_stats(tps);

    auto tes = current_state_.total_enemy_stats();
    tes.name = ai_player_->name();
    current_state_.set_total_enemy_stats(tes);
}

void Game::CleanUp() {
    current_state_.ResetForNewGame();
}


void Game::PauseGame() {
    if (current_state_.game_status() != GameStatus::PAUSED) {
        current_state_.set_game_status(GameStatus::PAUSED);
    } else if (current_state_.game_status() == GameStatus::PAUSED) {
        current_state_.set_game_status(GameStatus::PLAYER_TURN);
    }
}


void Game::RestartGame() {
    CleanUp();
    Initialize();
    current_state_.set_game_status(GameStatus::PLACING_SHIPS);
}



void Game::EndRound() {
    if (current_state_.game_status() == GameStatus::GAME_OVER) {
        return;
    }
    
    int round_result = current_state_.round_result();
    auto totalP = current_state_.total_player_stats();
    auto totalE = current_state_.total_enemy_stats();

   
    if (round_result == 1) {
        totalP.count_won++;
    } else if (round_result == 0) {
        totalE.count_won++;
    }
    totalP.rounds++;
    totalE.rounds++;
    current_state_.set_total_player_stats(totalP);
    current_state_.set_total_enemy_stats(totalE);

    current_state_.set_game_status(GameStatus::WAITING_NEXT_ROUND);
}

void Game::ExitGame() {
    current_state_.set_game_status(GameStatus::GAME_OVER);
}


void Game::set_cursor(int x, int y) {
    const PlayingField& target = enemy_field();
    int max_x = target.x_size();
    int max_y = target.y_size();
    if (max_x > 0 && max_y > 0) {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= max_x) x = max_x - 1;
        if (y >= max_y) y = max_y - 1;
    }
    current_state_.set_cursor(x, y);
}

void Game::MoveCursorBy(int dx, int dy) {
    const PlayingField& target = enemy_field();
    int max_x = target.x_size();
    int max_y = target.y_size();
    current_state_.MoveCursorBy(dx, dy, max_x, max_y);
}


void Game::ClearFields(){
    ai_player_->field_for_modification().ClearField();
    human_player_->field_for_modification().ClearField();
}

std::string Game::game_help() const {
    return 
        " ЦЕЛЬ ИГРЫ:\n"
        "   Первым уничтожить все корабли противника\n\n"

        " ПРАВИЛА:\n"
        "   • У каждого игрока есть флот из кораблей разных размеров\n"
        "   • Корабли размещаются на скрытом поле\n"
        "   • Игроки по очереди делают выстрелы по координатам\n"
        "   • Попадание отмечается на поле противника\n"
        "   • Сегмент корабля уничтожен, если получил 2 единицы урона\n"
        "   • Обычная атака наносит 1 единицу урона\n"
        "   • Компьютерный противник использует только обычную атаку\n"
        "   • Корабль считается уничтоженным, когда все его сегменты подбиты\n\n"

        " ПРОЦЕСС ИГРЫ:\n"
        "   1. Расстановка кораблей на своем поле\n"
        "   2. Поиск и уничтожение кораблей противника поочереди\n"
        "   3. Использование специальных способностей: двойной урон, скан, обстрел\n"
        "   4. Победа при полном уничтожении флота противника\n\n"

        " СПЕЦИАЛЬНЫЕ СПОСОБНОСТИ:\n"
        "   • В начале игры доступно 3 случайные способности\n"
        "   • После уничтожения корабля добавляется 1 случайная способность\n"
        "   • Double Damage - следующая атака наносит 2 урона\n"
        "   • Scanner - проверяет участок поля 2x2 на наличие кораблей\n"
        "   • Shelling - наносит урон случайному сегменту живого корабля противника\n\n"

        " СИСТЕМА СОХРАНЕНИЙ:\n"
        "   • Вы можете сохранить текущую игру в любой момент\n"
        "   • Загрузить ранее сохранённую игру для продолжения\n"
        "   • Статистика и прогресс также сохраняются\n\n"

        " УПРАВЛЕНИЕ ИГРОЙ:\n"
        "   • Можно начать/перезапустить раунд, сохранив или изменив настройки \n"
        "   • В настройках можно изменить размеры игрового поля или кораблей\n"
        "   • Игра состоит из последовательности раундов\n"
        "   • Статистика отображает результаты за все раунды";
}

void Game::TogglePlacementMode(){
    if (settings_.placement_mode() == PlacementMode::AUTO){
        settings_.set_placement_mode(PlacementMode::MANUAL);
    } else {
        show_ships_info_ = false;
        settings_.set_placement_mode(PlacementMode::AUTO);
    }
    ClearFields();
}




void Game::SaveStateForNextRound() {
    current_state_.set_player_field_state(human_player_->field());
    current_state_.set_enemy_field_state(ai_player_->field());
    current_state_.set_ship_manager(*ship_manager_);
}


void Game::LoadStateFromLastRound() {
    if (current_state_.round_number() <= 1) {
        current_state_.set_game_status(GameStatus::SETTING_SHIPS);
        return;
    }

    ship_manager_ = std::make_shared<ShipManager>(current_state_.ship_manager());

    int x_size = current_state_.player_field_state().x_size();
    int y_size = current_state_.player_field_state().y_size();

    human_player_ = std::make_unique<Player>("Игрок", PlayerType::HUMAN, ship_manager_, nullptr, x_size, y_size);
    ai_player_    = std::make_unique<Player>("ИИ",    PlayerType::AI,    ship_manager_, nullptr, x_size, y_size);

    human_player_->field_for_modification() = current_state_.player_field_state();
    ai_player_->field_for_modification()    = current_state_.enemy_field_state();

    human_player_->field_for_modification().ReturnStartState();
    ai_player_->field_for_modification().ReturnStartState();

    ability_manager_ = std::make_unique<AbilityManager>(ai_player_->field_for_modification(), human_player_->field_for_modification());
    human_player_->set_ability_manager(std::move(ability_manager_));

    current_state_.set_game_status(GameStatus::SETTING_SHIPS);
}

void Game::PrepareNextRound() {
    SaveStateForNextRound();                 
    current_state_.set_round_number(current_state_.round_number() + 1);
    LoadStateFromLastRound();
    current_state_.ResetForNewRound(); 
    
}



void Game::SaveGame(const std::string& filename) {  
    current_state_.set_player_field_state(human_player_->field());
    current_state_.set_enemy_field_state(ai_player_->field());
    current_state_.set_ship_manager(*ship_manager_); 
    current_state_.SaveGame(filename);
}

void Game::LoadGame(const std::string& filename) {
    try {
        GameState loaded_state;
        loaded_state.LoadGame(filename);
        current_state_.set_player_field_state(loaded_state.player_field_state());
        current_state_.set_enemy_field_state(loaded_state.enemy_field_state());
        current_state_.set_ship_manager(loaded_state.ship_manager());
        current_state_.set_player_turn(loaded_state.is_player_turn());
        current_state_.set_player_stats(loaded_state.player_stats());
        current_state_.set_enemy_stats(loaded_state.enemy_stats());
        current_state_.set_total_player_stats(loaded_state.total_player_stats());
        current_state_.set_total_enemy_stats(loaded_state.total_enemy_stats());
        current_state_.set_game_status(loaded_state.game_status());
        current_state_.set_round_number(loaded_state.round_number());
        current_state_.set_cursor(loaded_state.cursor_x(), loaded_state.cursor_y());
        LoadGameState();
    } catch (const std::bad_alloc&) {
        throw std::runtime_error("Некорректный формат файла сохранения");
    }
}

void Game::LoadGameState() {
    ship_manager_ = std::make_shared<ShipManager>(current_state_.ship_manager());

    int x_size = current_state_.player_field_state().x_size();
    int y_size = current_state_.player_field_state().y_size();

    human_player_ = std::make_unique<Player>("Игрок", PlayerType::HUMAN, ship_manager_, nullptr, x_size, y_size);
    ai_player_    = std::make_unique<Player>("ИИ",    PlayerType::AI,    ship_manager_, nullptr, x_size, y_size);

    human_player_->field_for_modification() = current_state_.player_field_state();
    ai_player_->field_for_modification()    = current_state_.enemy_field_state();

    
    ability_manager_ = std::make_unique<AbilityManager>(ai_player_->field_for_modification(), human_player_->field_for_modification());
    human_player_->set_ability_manager(std::move(ability_manager_));

    {
        auto ps = current_state_.player_stats();
        ps.name = human_player_->name();
        current_state_.set_player_stats(ps);

        auto es = current_state_.enemy_stats();
        es.name = ai_player_->name();
        current_state_.set_enemy_stats(es);
    }

    human_player_->set_destroyed_ships(current_state_.player_stats().destroyed);
    human_player_->set_hit_count(current_state_.player_stats().hits);
    human_player_->set_all_shots(current_state_.player_stats().shots);

    ai_player_->set_destroyed_ships(current_state_.enemy_stats().destroyed);
    ai_player_->set_hit_count(current_state_.enemy_stats().hits);
    ai_player_->set_all_shots(current_state_.enemy_stats().shots);
}


GameState&       Game::current_state()       { 
    return current_state_; 
}

const GameState& Game::current_state() const { 
    return current_state_; 
}

GameStatus Game::game_status() const { 
    return current_state_.game_status(); 
}

void Game::set_player_turn_status() { 
    current_state_.set_game_status(GameStatus::PLAYER_TURN); 
}

void Game::RotateShip() { 
    human_player_->RotateCurrentShip();
}

Orientation Game::ship_orientation() const { 
    return human_player_->current_orientation();
}

    
void Game::ClearShip(int x, int y){ 
    human_player_->field_for_modification().ClearShip(x, y); 
}

void Game::ToggleShipsInfo() { 
    show_ships_info_ = !show_ships_info_; 
}

bool Game::ShouldShowShipsInfo() const { 
    return show_ships_info_; 
}

void Game::ToggleHelp() {
    show_help_ = !show_help_; 
}

bool Game::ShouldShowHelp() const { 
    return show_help_; 
}

void Game::ToggleStats() { 
    show_stats_ = !show_stats_; 
}

bool Game::ShouldShowStats() const { 
    return show_stats_; 
}

void Game::set_game_status(GameStatus new_status) { 
    current_state_.set_game_status(new_status); 
}


void Game::AddShipSize(int size){
    settings_.AddShipSize(size);
}
   
void Game::ClearShipSizes(){
    settings_.set_fleet_mode(true);
    settings_.ClearTempFleetSpec();
}


void Game::set_ship_fleet_spec(){
    settings_.ApplyFleetSpec();
}


void Game::set_auto_ship_sizes(){
    settings_.set_fleet_mode();
}


int Game::temp_field_size() const {
    return settings_.temp_field_size();
}

void Game::ApplyFieldSize(){
    settings_.ApplyFieldSize();
}

void Game::set_temp_field_size(int size){
    settings_.set_temp_field_size(size);
}

    
std::string Game::fleet_spec_string(bool use_temp_fleet) const {
    std::vector<int> fleet_spec = use_temp_fleet ? settings_.temp_fleet_spec() : settings_.fleet_spec();
        
    if (fleet_spec.empty()) {
        return u8"Флот пуст";
    }
        
    std::sort(fleet_spec.begin(), fleet_spec.end(), std::greater<int>());
    std::stringstream ss;
    ss << u8"Корабли: ";
        
    std::map<int, int> ship_count_sizes;
    for (int size : fleet_spec) {
        ship_count_sizes[size]++;
    }
        
    bool first = true;
    for (const auto& [size, count] : ship_count_sizes) {
        if (!first) ss << u8", ";
        ss << count << u8"×" << size;
        first = false;
    }
        
    ss << u8" (всего: " << fleet_spec.size() << u8" кораблей)";
    return ss.str();
}



int Game::round_result() const {
    return current_state_.round_result();
}


int Game::round() const{
    return current_state_.round_number();
}

std::string Game::name() const { 
    return human_name_; 
}

std::string Game::ai_name() const { 
    return ai_player_ ? ai_player_->name() : std::string("AI"); 
}

int Game::player_score() const { 
    return human_player_ ? human_player_->destroyed_ships() : 0; 
}

int Game::enemy_score()  const { 
    return ai_player_ ? ai_player_->destroyed_ships() : 0; 
}

int Game::ship_count() const { 
    return ship_manager_->ship_count(); 
}

PlacementMode Game::placement_mode() const { 
    return settings_.placement_mode(); 
}



const PlayingField& Game::player_field() const & {
    return human_player_->field();
}

const PlayingField& Game::enemy_field() const & {
    return ai_player_->field();
}

bool Game::CanPlaceShip() const { 
    return (!human_player_->IsAllShipsPlaced());
}

int Game::cursor_x() const { 
    return current_state_.cursor_x(); 
}

int Game::cursor_y() const { 
    return current_state_.cursor_y(); 
}


bool Game::IsRunning() const {
    auto status = current_state_.game_status();
    return status != GameStatus::GAME_OVER && status != GameStatus::PLAYER_WON && status != GameStatus::ENEMY_WON;
}

bool Game::IsHelpClosed() { 
    return !show_help_; 
}

std::string Game::save_date() const { 
    return current_state_.save_date(); 
} 