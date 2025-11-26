#ifndef BATTLESHIP_CONTROLGAME_GAMESTATE_H_
#define BATTLESHIP_CONTROLGAME_GAMESTATE_H_

#include <vector>
#include <string>
#include <ostream>
#include <istream>
#include "core/PlayingField.h"
#include "abilities/AbilityManager.h"
#include "core/ShipManager.h"
#include <fstream>

struct PlayerStats {
    std::string name;      // Player's display name
    int hits;              // Number of successful Hits in current round
    int shots;             // Total shots fired in current round
    float accuracy;        // Hit accuracy percentage (Hits / shots)
    int destroyed;         // Number of enemy ships_ destroyed
    int remaining;         // Number of own ships_ still alive

    friend std::ostream& operator<<(std::ostream& os, const PlayerStats& stats);
    friend std::istream& operator>>(std::istream& is, PlayerStats& stats);
};

struct TotalPlayerStats {
    std::string name;      // Player's display name
    int total_hits = 0;    // Total Hits across all rounds
    int total_shots = 0;   // Total shots across all rounds
    float accuracy;        // Overall accuracy percentage
    int rounds = 0;        // Total rounds played
    int count_won = 0;     // Number of rounds won

    friend std::ostream& operator<<(std::ostream& os, const TotalPlayerStats& s);
    friend std::istream& operator>>(std::istream& is, TotalPlayerStats& s);
};

enum class GameStatus {
    PLACING_SHIPS,          // Placing ships_ on the field
    PLAYER_TURN,            // Player's turn to make a move
    ENEMY_TURN,             // Enemy's turn to make a move
    PLAYER_WON,             // Player won the current round
    ENEMY_WON,              // Enemy won the current round
    PAUSED,                 // Game is paused
    GAME_OVER,              // Game session ended
    SET_FIELD,              // Setting up playing field
    SET_SIZES,              // Configuring field sizes
    SET_PLACEMENT_MODE,     // Choosing ship placement mode
    SETTING_SHIPS,          // Actively placing ships_
    ASK_EXIT,               // Prompting for exit confirmation
    ASK_SAVE,               // Prompting for save confirmation
    WAITING_NEXT_ROUND,     // Waiting to start next round
    SELECT_SAVE_SLOT,       // Choosing save file slot
    SELECT_LOAD_SLOT        // Choosing load file slot
};

class GameState {
public:
    GameState();

    friend std::ostream& operator<<(std::ostream& os, const GameState& state);
    friend std::istream& operator>>(std::istream& is, GameState& state);

    // Persistence operations - handle game state serialization
    void SaveGame(const std::string& filename);
    void LoadGame(const std::string& filename);
    bool CanSave() const;
    bool CanLoad() const;

    void MoveCursorBy(int dx, int dy, int maxX, int maxY);
    
    void ResetForNewRound();
    void ResetForNewGame();

    ShipManager ship_manager() const;
    void set_ship_manager(const ShipManager& manager);

    GameStatus game_status() const;
    void set_game_status(GameStatus newstatus_);

    const PlayingField& player_field_state() const;
    void set_player_field_state(const PlayingField& state);

    const PlayingField& enemy_field_state()  const;
    void set_enemy_field_state(const PlayingField& state);

    const AbilityManager& player_abilities()  const;

    int round_number() const;
    void set_round_number(int round);
    
    int round_result() const;
    void set_round_result(int result);

    PlayerStats player_stats() const;  
    void set_player_stats(const PlayerStats& stats);
    
    PlayerStats enemy_stats() const;
    void set_enemy_stats(const PlayerStats& stats);

    TotalPlayerStats total_player_stats() const;  
    void set_total_player_stats(const TotalPlayerStats& stats);

    TotalPlayerStats total_enemy_stats() const;  
    void set_total_enemy_stats(const TotalPlayerStats& stats);

    std::string save_date() const;
    void set_save_date(const std::string& date);

    void set_cursor(int x, int y);
    int  cursor_x() const;
    int  cursor_y() const;
    
    int x_size();
    int y_size();

    std::string slot_date(const std::string& slotName) const;

    bool is_player_turn() const;
    void set_player_turn(bool turn);
    
private:
    PlayerStats player_stats_ = {"Игрок", 0, 0, 0.0f, 0, 0};
    PlayerStats enemy_stats_ = {"Противник", 0, 0, 0.0f, 0, 0};
    
    TotalPlayerStats total_player_stats_ = {"Игрок", 0, 0, 0.0f, 0, 0};
    TotalPlayerStats total_enemy_stats_ = {"Противник", 0, 0, 0.0f, 0, 0};
    
    GameStatus status_ = GameStatus::GAME_OVER;
    int round_result_ = -1; 
    bool is_player_turn_ = true;
    int round_number_ = 1;

    int cursor_x_ = 0;
    int cursor_y_ = 0; 

    std::string save_date_; 
    std::string save_directory_ = "saves/";

    PlayingField player_field_state_{10, 10};
    PlayingField enemy_field_state_{10, 10};
    
    AbilityManager player_abilities_{enemy_field_state_, player_field_state_};
    ShipManager ship_manager_; 
};

#endif
