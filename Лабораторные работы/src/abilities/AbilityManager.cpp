#include "AbilityManager.h"
#include "Ability.h"
#include "DoubleDamage.h"
#include "Scanner.h"
#include "Shelling.h"
#include "AbilityException.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <cctype>
#include <string>


static std::mt19937& rng() {
    static std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
    return gen;
}

std::shared_ptr<Ability> AbilityManager::GenerateRandomAbility() {
    std::uniform_int_distribution<int> dist(0, 2);
    switch (dist(rng())) {
        case 0: return std::make_shared<Scanner>();
        case 1: return std::make_shared<DoubleDamage>();
        case 2: return std::make_shared<Shelling>();
        default: return std::make_shared<Scanner>();
    }
}

AbilityManager::AbilityManager(PlayingField& enemy, PlayingField& self)
    : enemy_field_(enemy), field_(self) {
    InitializeThreeUniqueAbilities();
}

std::pair<int, int> AbilityManager::ApplyNextAbility(int x, int y) {
    if (ability_queue_.empty()) {
        throw EmptyQueueException();
    }
    auto ability = std::move(ability_queue_.front());
    ability_queue_.pop();
    ability->Use(*this, x, y);
    return ability->coordinates();
}

void AbilityManager::AddNextAbility() {
    ability_queue_.push(GenerateRandomAbility());
}

std::string AbilityManager::ability_name() const {
    if (ability_queue_.empty()) return "Нет способностей";
    return ability_queue_.front()->name();
}

std::string AbilityManager::PeekNextAbility() const {
    std::string name = ability_name();
    if (name != "Нет способностей") {
        std::string desc = ability_queue_.front()->description();
        return name + ": " + desc;
    } else {
        return name;
    }
}

size_t AbilityManager::queue_size() const {
    return ability_queue_.size();
}

int AbilityManager::enemy_field_size_x() const {
    return enemy_field_.x_size();
}

int AbilityManager::enemy_field_size_y() const {
    return enemy_field_.y_size();
}

void AbilityManager::DamageEnemyField(int x, int y, int dam) {
    if (enemy_field_.Damage(x, y, dam) == 2) {
        AddNextAbility();
    }
}

std::string AbilityManager::PrintAbilities() const {
    if (ability_queue_.empty()) {
        return "Нет доступных способностей.\n";
    }
    
    std::string result = "Доступные способности: ";
    std::queue<std::shared_ptr<Ability>> tmp = ability_queue_;
    
    bool first = true;
    while (!tmp.empty()) {
        auto& ability = tmp.front();
        if (ability) {
            if (!first) {
                result += ", ";
            }
            result += ability->name();
            first = false;
        }
        tmp.pop();
    }
    
    result += "\n";
    return result;
}


void AbilityManager::set_enemy_cell_visible(int x, int y) {
    enemy_field_.set_cell_visible(x, y);
}

bool AbilityManager::enemy_cell_state(int x, int y) const {
    return enemy_field_.IsShipCell(x, y);
}

const Ship& AbilityManager::ship(int index) const {
    return enemy_field_.ship(index);
}

void AbilityManager::InitializeThreeUniqueAbilities() {
    std::vector<std::shared_ptr<Ability>> abilities;
    abilities.push_back(std::make_shared<DoubleDamage>());
    abilities.push_back(std::make_shared<Scanner>());
    abilities.push_back(std::make_shared<Shelling>());

    std::shuffle(abilities.begin(), abilities.end(), rng());
    for (auto& a : abilities) {
        ability_queue_.push(a);
    }
}

void AbilityManager::clear() {
    while (!ability_queue_.empty()) ability_queue_.pop();
}

void AbilityManager::set_fields(PlayingField& enemy, PlayingField& self) {
    enemy_field_ = enemy;
    field_ = self;
}

void AbilityManager::set_ability_queue(std::queue<std::shared_ptr<Ability>> new_queue) {
    clear();
    ability_queue_ = std::move(new_queue);
}



int AbilityManager::ship_count() const {
    return enemy_field_.count();
}

std::ostream& operator<<(std::ostream& os, const AbilityManager& m) {
    auto tmp = m.ability_queue();
    os << tmp.size() << "\n";
    while (!tmp.empty()) {
        os << tmp.front()->name() << "\n";
        tmp.pop();
    }
    return os;
}

std::istream& operator>>(std::istream& is, AbilityManager& m) {
    size_t count_;
    if (!(is >> count_)) return is;
    std::string dummy;
    std::getline(is, dummy);

    m.clear();
    for (size_t i = 0; i < count_; ++i) {
        std::string name;
        std::getline(is, name);
        if (name.find("Double") != std::string::npos) {
            m.ability_queue_.push(std::make_shared<DoubleDamage>());
        } else if (name.find("Scanner") != std::string::npos) {
            m.ability_queue_.push(std::make_shared<Scanner>());
        } else if (name.find("Shelling") != std::string::npos) {
            m.ability_queue_.push(std::make_shared<Shelling>());
        }
    }
    return is;
}

void AbilityManager::reset() {
    clear();
    InitializeThreeUniqueAbilities();
}

std::queue<std::shared_ptr<Ability>> AbilityManager::ability_queue() const { 
    return ability_queue_; 
}

bool AbilityManager::HasAbilities() const {
    return !ability_queue_.empty(); 
}