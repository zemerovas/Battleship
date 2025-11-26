#ifndef BATTLESHIP_CORE_PLAYER_H_
#define BATTLESHIP_CORE_PLAYER_H_

#include <string>
#include <memory>
#include <iostream>
#include "PlayingField.h"
#include "ShipManager.h"
#include "AbilityManager.h"
#include "additional/Other.h"

enum class PlayerType {
    HUMAN,
    AI
};

class Player {
public:
    Player(const std::string& player_name, PlayerType player_type,
               std::shared_ptr<ShipManager> ship_manager,
               int field_width, int field_height);

    bool IsAllShipsDestroyed() const;
    bool IsAllShipsPlaced() const; 

    int MakeMove(std::unique_ptr<Player>& opponent, int x, int y);

    std::pair<int, int> UseAbility(int x, int y);
    
    void RotateCurrentShip();
    Orientation current_orientation();
    bool PlaceShipsRandomly();

    std::string name() const;
    PlayerType type() const;

    int destroyed_ships() const;
    void set_destroyed_ships(int count);

    int hit_count() const;
    void set_hit_count(int count);

    int all_shots() const;
    void set_all_shots(int count);

    std::shared_ptr<ShipManager> ship_manager() const;
    void set_ship_manager(std::shared_ptr<ShipManager> ship_manager);

    void set_ability_manager(std::shared_ptr<AbilityManager> ability_manager);

    float accuracy() const;
    int remaining_ships() const;

    const PlayingField& field() const &;
    PlayingField& field_for_modification() &;
    
private:
    std::string name_;
    PlayerType type_;
    std::unique_ptr<PlayingField> field_;
    std::shared_ptr<ShipManager> ship_manager_;
    std::shared_ptr<AbilityManager> ability_manager_;
    int destroyed_ships_;
    int hit_count_;
    int all_shots_;

};

#endif
