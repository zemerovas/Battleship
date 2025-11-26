#include "ShipManager.h"

ShipManager::ShipManager() : ship_count_(0) {}

ShipManager::ShipManager(int count, const std::vector<int>& sizes) : ship_count_(count), ship_sizes_(sizes) {}

ShipManager::ShipManager(const ShipManager& other): ship_count_(other.ship_count_), ship_sizes_(other.ship_sizes_) {}


ShipManager& ShipManager::operator=(const ShipManager& other) {
    if (this != &other) {
        ship_count_ = other.ship_count_;
        ship_sizes_ = other.ship_sizes_;
    }
    return *this;
}


std::ostream& operator<<(std::ostream& os, const ShipManager& manager) {
    os << manager.ship_count_ << '\n';
    os << manager.ship_sizes_.size() << '\n';
    for (int size : manager.ship_sizes_) {
        os << size << ' ';
    }
    os << '\n';
    return os;
}


std::istream& operator>>(std::istream& is, ShipManager& manager) {
    is >> manager.ship_count_;
    size_t sizes_count;
    is >> sizes_count;
    manager.ship_sizes_.resize(sizes_count);
    for (size_t i = 0; i < sizes_count; ++i) {
        is >> manager.ship_sizes_[i];
    }
    return is;
}


void ShipManager::clear() {
    ship_count_ = 0;
    ship_sizes_.clear();
}


int ShipManager::ship_count() const {
    return ship_count_;
}


int ShipManager::ship_size(int index) const {
    if (index >= ship_count_) {
        throw std::out_of_range("Индекс корабля вне диапазона");
    }
    return ship_sizes_[index];
}