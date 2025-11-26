#include "Ability.h"

Ability::Ability(const std::string& ability_name)
    : name_(ability_name), description_("Способность: " + ability_name), coord_(-1, -1) {}


std::string Ability::name() const { 
    return name_; 
}


std::string Ability::description() const { 
    return description_; 
}    


std::pair<int, int> Ability::coordinates() const { 
    return coord_; 
}