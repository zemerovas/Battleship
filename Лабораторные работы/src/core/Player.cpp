#include "Player.h"
#include "abilities/AbilityException.h"
#include "additional/ShipCoordinateExceptions.h"


Player::Player(const std::string& player_name, PlayerType player_type,
               std::shared_ptr<ShipManager> ship_manager,
               int field_width, int field_height)
    : name_(player_name),
      type_(player_type),
      field_(std::make_unique<PlayingField>(field_width, field_height)),
      ship_manager_(std::move(ship_manager)),
      destroyed_ships_(0),
      hit_count_(0),
      all_shots_(0) {}


bool Player::IsAllShipsDestroyed() const {
    return remaining_ships() == 0;
}


bool Player::IsAllShipsPlaced() const { 
    return ship_manager_->ship_count() == field_->count();
}


int Player::MakeMove(std::unique_ptr<Player>& opponent, int x, int y) {
    int x_size = field_->x_size();
    int y_size = field_->y_size();
    if (!IsValid(x, y, x_size, y_size)) {
        throw ShipOutOfBoundsException(x, y, x_size, y_size);
    }

    all_shots_++;
    int res = opponent->field_for_modification().Damage(x, y);

    if (res > 0) {
        hit_count_++;
        if (res == 2){
           destroyed_ships_++; 
           if (type_ == PlayerType::HUMAN && ability_manager_) {
                ability_manager_->AddNextAbility();
            }
        }
        
    }

    return res;
}



std::pair<int, int> Player::UseAbility(int x, int y) {
    if (!ability_manager_ || !ability_manager_->HasAbilities()) {
        throw EmptyQueueException();
    }
    return ability_manager_->ApplyNextAbility(x, y);
}


void Player::RotateCurrentShip(){
    field_->RotateOrientation();
}

Orientation Player::current_orientation(){
    return field_->current_orientation();
}

bool Player::PlaceShipsRandomly() {
    if (!ship_manager_) {
        return false;
    }
    return field_->SetRandomShips(*ship_manager_);
}

std::string Player::name() const { 
    return name_; 
}


PlayerType Player::type() const { 
    return type_; 
}


int Player::destroyed_ships() const { 
    return destroyed_ships_; 
}


void Player::set_destroyed_ships(int count) { 
    destroyed_ships_ = count; 
}


int Player::hit_count() const { 
    return hit_count_; 
}


void Player::set_hit_count(int count) { 
    hit_count_ = count; 
}


int Player::all_shots() const { 
    return all_shots_; 
}


void Player::set_all_shots(int count) { 
    all_shots_ = count; 
}


std::shared_ptr<ShipManager> Player::ship_manager() const { 
    return ship_manager_; 
}


void Player::set_ship_manager(std::shared_ptr<ShipManager> ship_manager) { 
    ship_manager_ = std::move(ship_manager); 
}


void Player::set_ability_manager(std::shared_ptr<AbilityManager> ability_manager) { 
    ability_manager_ = std::move(ability_manager); 
}


float Player::accuracy() const {
    if (all_shots_ == 0) return 0.0f;
    return static_cast<float>(hit_count_) / all_shots_ * 100.0f;
}


int Player::remaining_ships() const {
    return ship_manager_ ? (ship_manager_->ship_count() - destroyed_ships_) : 0;
}


const PlayingField& Player::field() const & {
    return *field_;
}

PlayingField& Player::field_for_modification() & {
    return *field_;
}
