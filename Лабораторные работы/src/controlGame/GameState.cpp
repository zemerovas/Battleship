#include "GameState.h"
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <ctime>
#include "FileHandler.h"

GameState::GameState() : ship_manager_() {  
    std::filesystem::create_directories(save_directory_);
}


std::ostream& operator<<(std::ostream& os, const PlayerStats& stats) {
    os << std::quoted(stats.name) << ' '
       << stats.hits << ' ' << stats.shots << ' ' << stats.accuracy << ' '
       << stats.destroyed << ' ' << stats.remaining << '\n';
    return os;
}


std::istream& operator>>(std::istream& is, PlayerStats& stats) {
    is >> std::quoted(stats.name)
       >> stats.hits >> stats.shots >> stats.accuracy
       >> stats.destroyed >> stats.remaining;
    return is;
}


std::ostream& operator<<(std::ostream& os, const TotalPlayerStats& s) {
    os << std::quoted(s.name) << ' '
       << s.total_hits << ' '
       << s.total_shots << ' '
       << s.accuracy << ' '
       << s.rounds << ' '
       << s.count_won << '\n';
    return os;
}


std::istream& operator>>(std::istream& is, TotalPlayerStats& s) {
    is >> std::quoted(s.name)
       >> s.total_hits
       >> s.total_shots
       >> s.accuracy
       >> s.rounds
       >> s.count_won;
    return is;
}


std::ostream& operator<<(std::ostream& os, const GameState& st) {
    os << std::quoted(st.save_date_) << '\n';
    os << static_cast<int>(st.status_) << '\n';
    os << st.round_result_ << ' ' << st.is_player_turn_ << '\n';
    os << st.round_number_ << '\n';
    os << st.cursor_x_ << ' ' << st.cursor_y_ << '\n';
    os << st.player_stats_;
    os << st.enemy_stats_;
    os << st.total_player_stats_;
    os << st.total_enemy_stats_;
    os << st.ship_manager_;
    os << st.player_field_state_.x_size() << ' ' << st.player_field_state_.y_size() << '\n';
    os << st.enemy_field_state_.x_size() << ' ' << st.enemy_field_state_.y_size() << '\n';
    os << st.player_field_state_;
    os << st.enemy_field_state_;
    os << st.player_abilities_;

    return os;
}


std::istream& operator>>(std::istream& is, GameState& st) {
    is >> std::quoted(st.save_date_);
    int status_value;
    is >> status_value;
    st.status_ = static_cast<GameStatus>(status_value);
    is >> st.round_result_ >> st.is_player_turn_;
    is >> st.round_number_;
    is >> st.cursor_x_ >> st.cursor_y_;
    is >> st.player_stats_ >> st.enemy_stats_;
    is >> st.total_player_stats_ >> st.total_enemy_stats_;
    is >> st.ship_manager_;

    int px, py, ex, ey;
    is >> px >> py >> ex >> ey;
    st.player_field_state_ = PlayingField(px, py);
    st.enemy_field_state_ = PlayingField(ex, ey);
    is >> st.player_field_state_ >> st.enemy_field_state_;
    is >> st.player_abilities_;

    return is;
}


void GameState::SaveGame(const std::string& filename) {
    std::string full_path = save_directory_ + filename + ".save";
    FileHandler file(full_path, std::ios::out | std::ios::trunc);
    if (!file.get()) return;

    time_t now = time(0);
    std::tm* local = std::localtime(&now);
    std::stringstream ss;
    ss << std::put_time(local, "%d.%m.%Y %H:%M");
    save_date_ = ss.str();

    file.get() << *this;
}


void GameState::LoadGame(const std::string& filename) {
    std::string full_path = save_directory_ + filename + ".save";
    FileHandler file(full_path, std::ios::in);
    if (!file.get()) return;

    file.get() >> *this;
}


bool GameState::CanSave() const {
    return status_ != GameStatus::ENEMY_TURN && status_ != GameStatus::ASK_SAVE && status_ != GameStatus::SET_FIELD && status_ != GameStatus::SET_SIZES && status_ != GameStatus::SELECT_LOAD_SLOT;
}


bool GameState::CanLoad() const {
    switch (status_) {
        case GameStatus::GAME_OVER:
        case GameStatus::PAUSED:
        case GameStatus::PLACING_SHIPS:
        case GameStatus::PLAYER_WON:
        case GameStatus::ENEMY_WON:
        case GameStatus::WAITING_NEXT_ROUND:
            return true;
        default:
            return false; 
    }
}


void GameState::MoveCursorBy(int dx, int dy, int max_x, int max_y) {
    if (max_x <= 0 || max_y <= 0) {
        cursor_x_ += dx;
        cursor_y_ += dy;
        if (cursor_x_ < 0) cursor_x_ = 0;
        if (cursor_y_ < 0) cursor_y_ = 0;
        return;
    }

    auto clamp = [](int v, int lo, int hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    };

    int nx = cursor_x_ + dx;
    int ny = cursor_y_ + dy;

    cursor_x_ = clamp(nx, 0, max_x - 1);
    cursor_y_ = clamp(ny, 0, max_y - 1);
}


void GameState::ResetForNewRound() {
    player_stats_ = PlayerStats{player_stats_.name, 0, 0, 0.0f, 0, 0};
    enemy_stats_ = PlayerStats{enemy_stats_.name, 0, 0, 0.0f, 0, 0};
    
    round_result_ = -1;
    is_player_turn_ = true;
    
    cursor_x_ = 0;
    cursor_y_ = 0;
        
    player_abilities_.reset();
}


void GameState::ResetForNewGame() {
    ResetForNewRound();
    round_number_ = 1;
        
    total_player_stats_ = TotalPlayerStats{total_player_stats_.name, 0, 0, 0.0f, 0, 0};
    total_enemy_stats_ = TotalPlayerStats{total_enemy_stats_.name, 0, 0, 0.0f, 0, 0};
}


ShipManager GameState::ship_manager() const { 
    return ship_manager_; 
}


void GameState::set_ship_manager(const ShipManager& manager) { 
    ship_manager_ = manager; 
}


GameStatus GameState::game_status() const { 
    return status_; 
}


void GameState::set_game_status(GameStatus new_status) { 
    status_ = new_status;
}


const PlayingField& GameState::player_field_state() const { 
    return player_field_state_; 
}


void GameState::set_player_field_state(const PlayingField& state) {
    player_field_state_ = state;
}


const PlayingField& GameState::enemy_field_state() const { 
    return enemy_field_state_; 
}


void GameState::set_enemy_field_state(const PlayingField& state) {
    enemy_field_state_ = state;
}


const AbilityManager& GameState::player_abilities() const { 
    return player_abilities_; 
}


int GameState::round_number() const { 
    return round_number_; 
}


void GameState::set_round_number(int round) { 
    round_number_ = round; 
}


int GameState::round_result() const {
    return round_result_;
}


void GameState::set_round_result(int result) {
    if (result >= -1 && result <= 1) {
        round_result_ = result;
    }
}


PlayerStats GameState::player_stats() const { 
    return player_stats_; 
}


void GameState::set_player_stats(const PlayerStats& stats) { 
    player_stats_ = stats; 
}


PlayerStats GameState::enemy_stats() const { 
    return enemy_stats_; 
}


void GameState::set_enemy_stats(const PlayerStats& stats) { 
    enemy_stats_ = stats; 
}


TotalPlayerStats GameState::total_player_stats() const { 
    return total_player_stats_;  
}


void GameState::set_total_player_stats(const TotalPlayerStats& stats) {
    total_player_stats_ = stats;
    total_player_stats_.accuracy = total_player_stats_.total_shots > 0
        ? (total_player_stats_.total_hits * 100.0f / total_player_stats_.total_shots)
        : 0.0f;
}


TotalPlayerStats GameState::total_enemy_stats() const { 
    return total_enemy_stats_; 
}


void GameState::set_total_enemy_stats(const TotalPlayerStats& stats) {
    total_enemy_stats_ = stats;
    total_enemy_stats_.accuracy = total_enemy_stats_.total_shots > 0
        ? (total_enemy_stats_.total_hits * 100.0f / total_enemy_stats_.total_shots)
        : 0.0f;
}


std::string GameState::save_date() const { 
    return save_date_; 
}


void GameState::set_save_date(const std::string& date) { 
    save_date_ = date; 
}

bool GameState::is_player_turn() const { 
    return is_player_turn_;
}

void GameState::set_player_turn(bool turn) { 
    is_player_turn_ = turn; 
}


void GameState::set_cursor(int x, int y) { 
    cursor_x_ = x; 
    cursor_y_ = y; 
}


int GameState::cursor_x() const { 
    return cursor_x_; 
}


int GameState::cursor_y() const { 
    return cursor_y_; 
}


int GameState::x_size() { 
    return player_field_state_.x_size();
}


int GameState::y_size() { 
    return player_field_state_.y_size();
}


std::string GameState::slot_date(const std::string& slot_name) const {
    std::string full_path = save_directory_ + slot_name+ ".save";
    std::ifstream file(full_path);
    if (!file.is_open()) return "";

    std::string date_line;
    std::getline(file, date_line);
    if (date_line.empty()) return "";

    if (date_line.front() == '"' && date_line.back() == '"' && date_line.size() >= 2)
        date_line = date_line.substr(1, date_line.size() - 2);

    return date_line;
}