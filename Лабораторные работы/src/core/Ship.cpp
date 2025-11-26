#include "Ship.h"
#include <stdexcept>


Ship::Ship(Position start, Orientation orient, int size, int number)
    : start_position_(start),
      ship_size_(size),
      ship_orientation_(orient),
      segments_(size, SegmentState::INTACT),
      ship_number_(number),
      destroyed_segments_(0),
      hit_count_(0) {
    if (size < 1 || size > 4) {
        throw std::invalid_argument("Размер корабля должен быть от 1 до 4");
    }
}

Ship::Ship(const Ship& other)
    : start_position_(other.start_position_),
      ship_size_(other.ship_size_),
      ship_orientation_(other.ship_orientation_),
      segments_(other.segments_),
      ship_number_(other.ship_number_),
      destroyed_segments_(other.destroyed_segments_),
      hit_count_(other.hit_count_) {}

Ship::Ship(Ship&& other) noexcept
    : start_position_(other.start_position_),
      ship_size_(other.ship_size_),
      ship_orientation_(other.ship_orientation_),
      segments_(std::move(other.segments_)),
      ship_number_(other.ship_number_),
      destroyed_segments_(other.destroyed_segments_),
      hit_count_(other.hit_count_) {}

Ship& Ship::operator=(const Ship& other) {
    if (this != &other) {
        start_position_ = other.start_position_;
        ship_size_ = other.ship_size_;
        ship_orientation_ = other.ship_orientation_;
        segments_ = other.segments_;
        ship_number_ = other.ship_number_;
        destroyed_segments_ = other.destroyed_segments_;
        hit_count_ = other.hit_count_;
    }
    return *this;
}

Ship& Ship::operator=(Ship&& other) noexcept {
    if (this != &other) {
        start_position_ = other.start_position_;
        ship_size_ = other.ship_size_;
        ship_orientation_ = other.ship_orientation_;
        segments_ = std::move(other.segments_);
        ship_number_ = other.ship_number_;
        destroyed_segments_ = other.destroyed_segments_;
        hit_count_ = other.hit_count_;
    }
    return *this;
}


Ship::~Ship() {}


int Ship::DamageShip(Position position_for_damage, int damage) {
    int index = segment_index(position_for_damage);
    if (index < 0 || index >= ship_size_) {
        return -1;
    }
    if (segments_[index] == SegmentState::DESTROYED) {
        return -1;
    }
    Hit(index, damage);
    if (IsDestroyed()) {
        return 1;
    }
    return 2;
}


void Ship::Hit(int index, int damage) {
    if (segments_[index] == SegmentState::INTACT) {
        segments_[index] = (damage >= 2) ? SegmentState::DESTROYED : SegmentState::DAMAGED;
        destroyed_segments_++; 
        hit_count_++;
        return;
    }

    if (segments_[index] == SegmentState::DAMAGED) {
        segments_[index] = SegmentState::DESTROYED;
        hit_count_++;
        return;
    }
}


bool Ship::IsDestroyed() const {
    for (auto s : segments_) {
        if (s != SegmentState::DESTROYED) return false;
    }
    return true;
}


void Ship::MarkFullyDestroyed() {
    for (auto& s : segments_) s = SegmentState::DESTROYED;
    destroyed_segments_ = ship_size_;
}


int Ship::ship_size() const { 
    return ship_size_; 
}


int Ship::segment_index(Position current) const {
    if (ship_orientation_ == Orientation::HORIZONTAL) {
        if (current.y != start_position_.y) return -1;
        int index = current.x - start_position_.x;
        return (index >= 0 && index < ship_size_) ? index : -1;
    } else {
        if (current.x != start_position_.x) return -1;
        int index = current.y - start_position_.y;
        return (index >= 0 && index < ship_size_) ? index : -1;
    }
}


Position Ship::segment_position(int index) const {
    if (!(0 <= index && index < ship_size_)) {
        return Position(-1, -1);
    }
    if (ship_orientation_ == Orientation::HORIZONTAL) {
        return Position(start_position_.x + index, start_position_.y);
    } else {
        return Position(start_position_.x, start_position_.y + index);
    }
}


const std::vector<SegmentState>& Ship::segments() const { 
    return segments_; 
}


Position Ship::start_position() const { 
    return start_position_; 
}


void Ship::set_start_position(Position pos){
    start_position_ = pos;
}


Orientation Ship::orientation() const { 
    return ship_orientation_; 
}


void Ship::set_orientation(Orientation orientation){
    ship_orientation_ = orientation;
}


int Ship::ship_number() const { 
    return ship_number_; 
}


void Ship::set_ship_number(int number) { 
    ship_number_ = number; 
}


SegmentState Ship::segment_state(int index) const {
    if (index >= 0 && index < static_cast<int>(segments_.size())) {
        return segments_[index];
    }
    throw std::out_of_range("Сегмент с индексом " + std::to_string(index) + " вне диапазона");
}


void Ship::set_segment_state(int index, SegmentState state){
    segments_[index] = state;
}


int Ship::destroyed_segments() const { 
    return destroyed_segments_; 
}


void Ship::set_destroyed_segments(int count){
    destroyed_segments_ = count;
}


int Ship::hit_count() const { 
    return hit_count_; 
}


void Ship::set_hit_count(int count) { 
    hit_count_ = count; 
}


std::tuple<int, SegmentState, Orientation> Ship::segment_state(int x, int y) const {
    int index = segment_index(Position(x, y));
    if (index != -1) {
        return std::make_tuple(index, segments_[index], ship_orientation_);
    }
    return std::make_tuple(-1, SegmentState::NONE, Orientation::HORIZONTAL);
}
