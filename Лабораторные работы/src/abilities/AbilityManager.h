#ifndef BATTLESHIP_ABILITIES_ABILITYMANAGER_H_
#define BATTLESHIP_ABILITIES_ABILITYMANAGER_H_

#include <queue>
#include <memory>
#include <string>
#include <utility>
#include <iostream>

class Ability; 
class PlayingField;
class Ship;

class AbilityManager {
public:
    AbilityManager() = default;
    AbilityManager(PlayingField& enemy, PlayingField& self);

    std::pair<int, int> ApplyNextAbility(int x, int y); 
    void AddNextAbility();
    void InitializeThreeUniqueAbilities();
    void reset(); 

    bool HasAbilities() const;
    std::string PeekNextAbility() const;
    size_t queue_size() const;
    std::string PrintAbilities() const;

    std::string ability_name() const;    
    const Ship& ship(int index) const;
    bool enemy_cell_state(int x, int y) const;
    int enemy_field_size_x() const;
    int enemy_field_size_y() const;
    int ship_count() const;
    std::queue<std::shared_ptr<Ability>> ability_queue() const;

    void DamageEnemyField(int x, int y, int dam = 1);
    void set_enemy_cell_visible(int x, int y);

    void set_fields(PlayingField& enemy, PlayingField& self);
    void set_ability_queue(std::queue<std::shared_ptr<Ability>> new_queue);

    void clear();
    

    friend std::ostream& operator<<(std::ostream& os, const AbilityManager& m);
    friend std::istream& operator>>(std::istream& is, AbilityManager& m);

private:
    std::shared_ptr<Ability> GenerateRandomAbility();

    std::queue<std::shared_ptr<Ability>> ability_queue_;
    PlayingField& enemy_field_ ;
    PlayingField& field_ ;
};

#endif