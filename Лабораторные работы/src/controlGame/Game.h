#ifndef BATTLESHIP_CONTROLGAME_GAME_H_
#define BATTLESHIP_CONTROLGAME_GAME_H_

#include "GameState.h"
#include <iomanip>
#include <thread>
#include <chrono>
#include <memory>
#include <string>
#include "core/PlayingField.h"
#include "abilities/AbilityManager.h"
#include "core/ShipManager.h"
#include "core/Player.h"
#include "additional/Other.h"
#include "Result.h"
#include "GameSettings.h"
#include <map>

class Player;

struct ShipDisplayInfo {
    int number;       
    int size;        
    bool is_placed;
    Position start_pos;
    Orientation orientation;
};

class Game {
public:
    Game(GameSettings newSet);
    ~Game();

    void CleanUp();
    void Initialize();
    void CreateShipManager();

    void MoveAIShips(); 
    void MoveRandomShips();
    void RotateShip();
    
    void ClearShip(int x, int y);
    void MoveShip(int x, int y, Orientation orientation);
    void ClearFields();
    void TogglePlacementMode();

    AttackResult MakeAIMove();
    AttackResult AttackShip();
    AttackResult AttackShipAt(int x, int y);
    AbilityResult UseAbility(int x, int y);
    void ToggleShipsInfo();
    bool ShouldShowShipsInfo() const;
    void ToggleHelp();
    bool ShouldShowHelp() const;
    void ToggleStats();
    bool ShouldShowStats() const;
    
    void AddShipSize(int size);
    void ClearShipSizes();
    void ApplyFieldSize();

    void UpdateScore();
    void CheckWinCondition();
    void LoadGameState();
    void SaveStateForNextRound();
    void LoadStateFromLastRound();
    void PrepareNextRound();

    void PauseGame();
    void RestartGame();
    void EndRound();
    void ExitGame();
    void SaveGame(const std::string& filename);
    void LoadGame(const std::string& filename);
    std::string ShowAbility() const;
    
    bool CanSaveGame() const;
    bool CanLoadGame() const;
    bool IsNotRunning();

    const PlayingField& player_field() const &;
    const PlayingField& enemy_field() const &;
    bool CanPlaceShip() const;
    void UpdateTotalStats();
    
    void MoveCursorBy(int dx, int dy);
    bool IsRunning() const;
    bool IsHelpClosed();

    std::string name()   const;
    std::string ai_name() const;

    GameState&       current_state();
    const GameState& current_state() const;

    void set_players(std::unique_ptr<Player> human, std::unique_ptr<Player> ai);
    int player_score() const;
    int enemy_score()  const;
    Orientation ship_orientation() const;
    int current_ship_size() const;
    int cursor_x() const;
    int cursor_y() const;
    void set_cursor(int x, int y);
    int ship_count() const;
    int round_result() const;
    int round() const;
    PlacementMode placement_mode() const;
    std::string game_help() const;  
    std::string save_date() const;
    GameStatus game_status() const;
    void set_game_status(GameStatus newStat);
    void set_player_turn_status();
    std::pair<int, Orientation> current_ship_info() const;
    void set_temp_field_size(int size);    
    int temp_field_size() const;
    std::string statistics() const;
    std::vector<ShipDisplayInfo> human_player_ships_info() const;
    std::string fleet_spec_string(bool use_temp_fleet = false) const;
    void set_ship_fleet_spec();
    void set_auto_ship_sizes();

private:
    std::unique_ptr<Player> human_player_;
    std::unique_ptr<Player> ai_player_;
    std::shared_ptr<ShipManager> ship_manager_;
    std::shared_ptr<AbilityManager> ability_manager_;
    std::string human_name_;
    GameState current_state_;
    GameSettings settings_;
    bool show_ships_info_ = false;
    bool show_help_ = false;
    bool show_stats_ = false;
};

#endif
